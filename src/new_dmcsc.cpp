/* DMCS -- Distributed Nonmonotonic Multi-Context Systems.
 * Copyright (C) 2009, 2010 Minh Dao-Tran, Thomas Krennwallner
 * 
 * This file is part of DMCS.
 *
 *  DMCS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DMCS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DMCS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   new_dmcsc.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Mar  22 21:45:12 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "dmcs/ProgramOptions.h"
#include "mcs/BackwardMessage.h"
#include "mcs/BeliefTable.h"
#include "mcs/ID.h"
#include "mcs/NewBeliefState.h"
#include "mcs/ForwardMessage.h"
#include "mcs/Logger.h"
#include "mcs/QueryID.h"
#include "mcs/ReturnedBeliefState.h"
#include "network/NewClient.h"
#include "mcs/QueryPlan.h"
#include "parser/NewQueryPlanParser.hpp"

#include "parser/Parser.hpp"
#include "parser/Parser.tcc"
#include "parser/QueryPlanGrammar.hpp"
#include "parser/QueryPlanGrammar.tcc"

#include <set>
#include <fstream>
#include <iostream>
#include <set>
#include <string> 
#include <csignal>

using namespace dmcs;

std::set<NewBeliefState> final_result;
std::size_t no_belief_states = 0;
std::size_t handled_belief_states = 0;
std::size_t system_size = 0;
std::size_t bs_size = 0;
ContextQueryPlanMapPtr query_plan = ContextQueryPlanMapPtr();

boost::posix_time::ptime start_time;
boost::mutex print_mutex;


void
print_belief_state(const NewBeliefState& bs)
{
  std::size_t count = bs.status_bit.count();
  if (count == 0)
    {
      std::cout << "[ ]";
    }

  std::cout << "[";
  std::size_t bit = bs.status_bit.get_first();
  do
    {
      if (!bs.value_bit.test(bit)) std::cout << "-";

      std::size_t ctx_id = bit / (bs_size+1);
      std::size_t local_pos = bit % (bs_size+1);

      const ContextQueryPlan& cqp = query_plan->find(ctx_id)->second;
      const BeliefTablePtr& signature = cqp.localSignature;

      if (local_pos > 0) // non-epsilon bit
	{
	  ID local_id(ID::MAINKIND_BELIEF, local_pos);
	  const Belief& b = signature->getByID(local_id);

	  std::cout << ctx_id << ":" << b.text;
	}

      bit = bs.status_bit.get_next(bit);
      
      if (local_pos > 0 && bit) std::cout << ", ";
    }
  while (bit);
}


void
handle_belief_states(ReturnedBeliefStateListPtr result)
{
  for (ReturnedBeliefStateList::iterator it = result->begin();
       it != result->end(); ++it)
    {
      NewBeliefState* belief_state = (*it)->belief_state;
      if (belief_state)
	{
	  std::pair<std::set<NewBeliefState>::iterator, bool> p = final_result.insert(*belief_state);
	  if (p.second)
	    {
	      no_belief_states++;

	      boost::mutex::scoped_lock lock(print_mutex);

	      // query_plan is global
	      if (query_plan != ContextQueryPlanMapPtr())
		{
		  std::cout << "Partial Equilibrium #" << no_belief_states << ": ( ";
		  print_belief_state(*p.first);
		  std::cout << ")" << std::endl;
		}
	      else
		{
		  std::cout << "Partial Equilibrium #" << no_belief_states << ": ( " << *p.first << ")" << std::endl;
		}

	      //boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - start_time;

	      //std::cerr << "[" << diff.total_seconds() << "." << diff.total_milliseconds() << "] "
	      //	<< "Partial Equilibrium #" << no_belief_states << ": ( " << *p.first << ")" << std::endl;
	      
	    }

	  delete belief_state;
	  belief_state = 0;
	}
    }

  handled_belief_states += result->size();
}



void
handle_signal(int signum)
{
  if (signum == SIGINT) // interrupted, print current stats
    {
      boost::mutex::scoped_lock lock(print_mutex);

      std::cerr << "Total Number of Received Equilibria: " << handled_belief_states << std::endl;

      std::cout << "Total Number of Equilibria: " << no_belief_states << "+" << std::endl;

      exit(0);
    }
}



struct SamplingThread
{
  void
  operator() (int msecs)
  {
    while (1)
      {
	boost::this_thread::sleep(boost::posix_time::milliseconds(msecs));

	{
	  boost::mutex::scoped_lock lock(print_mutex);

	  boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - start_time;

	  std::cerr << "[" << diff.total_seconds() << "." << diff.total_milliseconds() << "] "
		    << "Received partial equilibria: " << handled_belief_states << std::endl;
	}
      }
  }

};


int 
main(int argc, char* argv[])
{
  try
    {
      std::string hostname;
      std::string port;
      std::string filename_signature = "";
      std::size_t root_ctx;
      bool loop;
      // range of requested equilibria. [0,0] for requesting all equilibria
      std::size_t k1;
      std::size_t k2;

      const char* help_description = "\nUsage: dmcsc --hostname=HOSTNAME --port=PORT [OPTIONS]\n\nOptions";

      boost::program_options::options_description desc(help_description);

      desc.add_options()
	(HELP, "produce help and usage message")
	(HOSTNAME, boost::program_options::value<std::string>(&hostname)->default_value("localhost"), "set host name")
	(PORT, boost::program_options::value<std::string>(&port), "set port")
	(ROOT_CTX, boost::program_options::value<std::size_t>(&root_ctx)->default_value(0), "set root context id to query")
	(SIGNATURE, boost::program_options::value<std::string>(&filename_signature)->default_value(""), "set signature file name")
	(BS_SIZE, boost::program_options::value<std::size_t>(&bs_size), "set belief state size")
	(LOOP, boost::program_options::value<bool>(&loop)->default_value(true), "set belief state size")
	(K1, boost::program_options::value<std::size_t>(&k1)->default_value(1), "set starting range of requested equlibria. k1 <= k2")
	(K2, boost::program_options::value<std::size_t>(&k2)->default_value(1), "set end range of requested equilibria. [0,0] for requesting all equilibria")
	;

      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    

      if (vm.count(HELP))
	{
	  std::cerr << desc << std::endl;
	  exit(1);
        }

      if (k1 > k2 || (k1 == 0 && k2 != 0))
	{
	  std::cerr << "Invalid range of requested equilibria." << std::endl;
	  std::cerr << desc << std::endl;
	  return 1;
	}

      std::size_t pack_size = 0;
      if (k1 > 0)
	{
	  pack_size = k2 - k1 + 1;
	}

      // setup connection
      boost::shared_ptr<boost::asio::io_service> io_service(new boost::asio::io_service);
      boost::asio::ip::tcp::resolver resolver(*io_service);
      boost::asio::ip::tcp::resolver::query query(hostname, port);
      boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
      boost::asio::ip::tcp::endpoint endpoint = *it;

      // setup client
      std::string header = HEADER_REQ_DMCS;
      std::size_t invoker = INVOKER_ID;      
      std::size_t query_order = 1;
      std::size_t qid = query_id(invoker, root_ctx, query_order);
      std::size_t end_qid = shutdown_query_id(invoker, root_ctx);
      NewHistory history;
      history.insert(INVOKER_ID);
      ForwardMessage request(qid, history, k1, k2);
      ForwardMessage end_message(end_qid, history);

      NewClient client(*io_service, it, header, request, loop);
      client.setCallback(&handle_belief_states);

      if (!filename_signature.empty())
	{
	  if (bs_size == 0)
	    {
	      std::cerr << "When signature file name is available, the following parameters must be set:" << std::endl
			<< "  --belief-state-size " << std::endl;
	      std::cerr << desc << std::endl;
	      return 1;
	    }
	  QueryPlanParser_t queryplan_parser;
	  query_plan = queryplan_parser.parseFile(filename_signature);
	}

      // catch Ctrl-C and interrupts
      sig_t s = signal(SIGINT, handle_signal);
      if (s == SIG_ERR)
	{
	  perror("signal");
	  exit(1);
	}

      bool keep_running = true;
      bool last_round = false;
      std::size_t next_count = 0;
      std::size_t model_count = 0;
      std::size_t last_model_count = 0;
      std::size_t diff_count = 0;

      SamplingThread sampler;
            
      // start iterating
      start_time = boost::posix_time::microsec_clock::local_time();
            
      boost::thread sampler_thread(sampler, 1000);

      while (keep_running)
	{
	  DBGLOG(DBG, "new_dmcsc: Entering round " << next_count);
	  io_service->run();

	  last_model_count = model_count;
	  model_count = handled_belief_states;

	  diff_count = final_result.size() - diff_count;
	  next_count++;

	  if (!loop)
	    {
	      DBGLOG(DBG, "new_dmcsc: Do not require looping. Going to terminate the client after 1st round.");
	      client.terminate(end_message);
	      break;
	    }

	  if (last_round)
	    {
	      keep_running = false;
	    }
	  else
	    {
	      io_service->reset();

	      if (diff_count > 0 && pack_size > final_result.size())
		{
		  std::size_t left_to_request = pack_size - final_result.size();
		  k1 = k2 + 1;
		  k2 = k1 + left_to_request;
		  DBGLOG(DBG, "new_dmcsc: Going to ask for next. k1 = " << k1 << ", k2 = " << k2);
		  request.setPackRequest(k1, k2);
		  client.next(request);
		}
	      else 
		// either fixed point reached 
		// or all equilibria were requested, i.e., [k1,k2] = [0,0]
		{
		  DBGLOG(DBG, "new_dmcsc: Going to terminate the client.");
		  client.terminate(end_message);
		  last_round = true;
		}
	    }
	} // end while (keep_running)

      std::cout << "Total Number of Equilibria: " << final_result.size() << std::endl;

      sampler_thread.interrupt();
      if (sampler_thread.joinable())
	{
	  sampler_thread.join();
	}
    }
  catch (std::exception& e)
    {
      DBGLOG(ERROR, "Bailing out: " << e.what());
      return 1;
    }
  catch (...)
    {
      DBGLOG(ERROR, "Unknown exception!");
      return 1;
    }
  
  return 0;
}

// Local Variables:
// mode: C++
// End:
