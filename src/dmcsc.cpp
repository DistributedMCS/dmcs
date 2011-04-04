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
 * @file   dmcsc.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Fri Jan  8 17:19:15 2010
 * 
 * @brief  
 * 
 * 
 */


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "dmcs/Log.h"
#include "dmcs/OptCommandType.h"
#include "dmcs/PrimitiveCommandType.h"
#include "dmcs/StreamingCommandType.h"

#include "dyndmcs/DynamicCommandType.h"
#include "dyndmcs/DynamicConfiguration.h"
#include "dyndmcs/InstantiatorCommandType.h"

#include "network/AsynClient.h"
#include "network/Client.h"

#include "dmcs/Message.h"
#include "dmcs/QueryPlan.h"
#include "dmcs/ProgramOptions.h"

#include "mcs/Theory.h"
#include "mcs/BeliefState.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <set>
#include <string> 
#include <csignal>


using namespace dmcs;


//@todo: can we reuse endpoint, io_service from main()?
void
instantiate(ContextSubstitutionPtr ctx_sub, const std::string& hostName, const std::string& port)
{
  // tell the root context to start the instantiation process with the
  // substitution ctx_sub
  
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(hostName, port);
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *it;

  std::string header = HEADER_REQ_INSTANTIATE;
  InstantiateForwardMessage mess(ctx_sub);
  Client<InstantiatorCommandType> c(io_service, it, header, mess);
  io_service.run();

  InstantiateBackwardMessagePtr answer = c.getResult();

  if (answer->getStatus() == true)
    {
      DMCS_LOG_DEBUG("Instantiation finished successfully!");

      // Now call the evaluation
    }
}

std::size_t no_beliefstates = 0;
bool complete = false;

std::set<PartialBeliefState> final_result;
std::size_t handled_belief_states = 0;

boost::posix_time::ptime start_time;

boost::mutex print_mutex;


void
handle_belief_state(StreamingBackwardMessage& m)
{
#ifdef DEBUG
  std::cerr << "recvd: " << m << std::endl;
#endif

  const ModelSessionIdListPtr& result = m.getResult();

  for (ModelSessionIdList::iterator it = result->begin(); it != result->end(); ++it)
    {
      PartialBeliefState* pbs = it->partial_belief_state;
      if (pbs != 0)
	{
	  std::pair<std::set<PartialBeliefState>::iterator,bool> p = final_result.insert(*pbs);

	  if (p.second)
	    {
	      no_beliefstates++;

	      boost::mutex::scoped_lock lock(print_mutex);

	      std::cout << "Partial Equilibrium #" << no_beliefstates << ": ( " << *p.first << ")" << std::endl;

	      boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - start_time;

	      std::cerr << "[" << diff.total_seconds() << "." << diff.total_milliseconds() << "] "
			<< "Partial Equilibrium #" << no_beliefstates << ": ( " << *p.first << ")" << std::endl;
	    }

	  delete pbs;
	  pbs = 0;
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

      std::cout << "Total Number of Equilibria: " << no_beliefstates << "+" << std::endl;

      exit(0);
    }
#if 0
  else if (signum == SIGALRM)
    {
      std::cerr << "Timeout: " << timeout << std::endl;
    }
#endif
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
      std::string hostName;
      std::string port;
      std::string manager;
      std::string qvs;
      std::size_t system_size = 0;
      std::size_t root_ctx = 0;
      std::size_t pack_size = 0;
      bool dynamic = false;
      bool primitiveDMCS = false;
      bool streaming = false;
      BeliefStatePtr V(new BeliefState);

      bool all_answers = false;
      
      const char* help_description = "\
dmcsc " PACKAGE_VERSION " ---"
#ifdef DEBUG
	" DEBUG"
#else
#ifdef NDEBUG
	" RELEASE"
#else
	" "
#endif // NDEBUG
#endif // DEBUG
#ifdef DMCS_STATS_INFO
	"STATS"
#else
	""
#endif // DMCS_STATS_INFO
"\n\n\
Usage: dmcsc --hostname=NAME --port=PORT --system-size=N [OPTIONS]\n\
\n\
Options";


      boost::program_options::options_description desc(help_description);

      desc.add_options()
	(HELP, "produce help and usage message")
	(HOSTNAME, boost::program_options::value<std::string>(&hostName)->default_value("localhost"), "set host name")
	(PORT, boost::program_options::value<std::string>(&port), "set port")
	(QUERY_VARS, boost::program_options::value<std::string>(&qvs), "set query variables")
	(SYSTEM_SIZE, boost::program_options::value<std::size_t>(&system_size), "set system size")
	(MANAGER, boost::program_options::value<std::string>(&manager), "set Manager HOST:PORT")
	(DYNAMIC, boost::program_options::value<bool>(&dynamic)->default_value(false), "set to dynamic mode")
	(ROOT_CTX, boost::program_options::value<std::size_t>(&root_ctx)->default_value(1), "set root context id")
	(STREAMING, boost::program_options::value<bool>(&streaming)->default_value(true), "set streaming mode")
	(PACK_SIZE, boost::program_options::value<std::size_t>(&pack_size)->default_value(1), "set number of belief states returned in one pack")
	;
	
      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    

      if (vm.count(HELP))
	{
	  std::cerr << desc << std::endl;
	  exit(1);
        }

      // setup log4cxx
      init_loggers("dmcsc");
      
      if (!qvs.empty()) // reading V for basic DMCS
	{
	  primitiveDMCS = true;
	  std::istringstream iss(qvs);
	  iss >> V;
	}

      if (port.empty() || hostName.empty())
	{
	  std::cerr << "Need hostname and port." << std::endl;
	  std::cerr << desc << std::endl;
	  exit(1);
	}

      // setup connection
      boost::shared_ptr<boost::asio::io_service> io_service(new boost::asio::io_service);
      boost::asio::ip::tcp::resolver resolver(*io_service);
      boost::asio::ip::tcp::resolver::query query(hostName, port);
      boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
      boost::asio::ip::tcp::endpoint endpoint = *it;



      if (dynamic) // dynamic mode
	{
	  DMCS_LOG_DEBUG("In dynamic mode.");

	  ContextSubstitutionPtr ctx_sub(new ContextSubstitution);
	  ConfigMessage mess(root_ctx, ctx_sub, false);
	  
	  DMCS_LOG_DEBUG("Message = " << mess);
	  
	  std::string header = HEADER_REQ_DYN_DMCS;
	  Client<DynamicCommandType> c(*io_service, it, header, mess);
	  io_service->run();

	  DynamicConfiguration::dynmcs_return_type result = c.getResult();

	  DMCS_LOG_DEBUG("FINAL RESULT: ");
	  DMCS_LOG_DEBUG(*result);

	  std::cerr << "FINAL RESULT:\n" << *result << std::endl;

	  //if (result->size() > 0)
	  //  {
	      // instantiate the system, then write .br and .sh files
	  //    ContextSubstitutionList::const_iterator it = result->begin();
	  //    instantiate(*it, hostName, port);
	  //  }
	}
      else // ground mode
	{
	  if(system_size == 0 ||
	     (primitiveDMCS && V->size() == 0) ||
	     (primitiveDMCS && V->size() != system_size))
	    {
	      std::cerr << "empty system size" << std::endl;
	      std::cerr << desc << std::endl;
	      return 1;
	    }

	  // our result
#ifdef DMCS_STATS_INFO
	  ReturnMessagePtr result(new ReturnMessage);
#else
	  BeliefStateListPtr result(new BeliefStateList);
#endif // DMCS_STATS_INFO
	  
	  DMCS_LOG_DEBUG("Starting DMCS with " << system_size << " contexts.");
	  
	  if (primitiveDMCS) // primitive DMCS
	    {
	      DMCS_LOG_DEBUG("Primitive mode.");
	      DMCS_LOG_DEBUG("Sending: " << V);

	      std::string header = HEADER_REQ_PRI_DMCS;
	      PrimitiveMessage mess(V);
	      Client<PrimitiveCommandType> c(*io_service, it, header, mess);
	      
	      DMCS_LOG_DEBUG("Running ioservice.");
	  
	      io_service->run();

	      DMCS_LOG_DEBUG("Getting results.");
	  
	      result = c.getResult();

	      all_answers = true;

#ifdef DMCS_STATS_INFO
	      no_beliefstates = result->getBeliefStates()->size();
#else
	      no_beliefstates = result->size();
#endif
	    }
	  else // opt or streaming DMCS
	    {
	      // for now, we work on streaming DMCS for the opt topology. 
	      if (streaming)
		{
		  DMCS_LOG_DEBUG("Streaming mode.");

		  if (pack_size == 0)
		    {
		      //StreamingCommandType::input_type mess(0, 0, 0, 1, pack_size);
		      
		      //if (pack_size == 0)
		      //{
		      // mess.setPackRequest(0, 0);
		      //}

		      StreamingCommandType::input_type mess(0, 0, 0, 0, 0);
		      
		      std::string header = HEADER_REQ_STM_DMCS;
		      
		      AsynClient<StreamingForwardMessage, StreamingBackwardMessage> c(*io_service, it, header, mess);
		      
		      c.setCallback(&handle_belief_state);
		      
		      io_service->run(); // wait for one round
		      c.terminate();
		      
		      no_beliefstates = final_result.size();
		    }
		  else
		    {
		      std::size_t k = pack_size > 0 ? pack_size : 10; // default: k is 10
		      complete = pack_size > 0 ? false : true;		  
		      
		      // USER <--> session_id = 0, path = {0}, invoker = 0
		      //#ifdef DEBUG
		      //History path(1, 0);
		      //StreamingCommandType::input_type mess(path, 0, 0, 1, k);
		      //#else
		      StreamingCommandType::input_type mess(0, 0, 0, 1, k);
		      //#endif
		      
		      std::string header = HEADER_REQ_STM_DMCS;
		      
		      AsynClient<StreamingForwardMessage, StreamingBackwardMessage> c(*io_service, it, header, mess);
		      
		      c.setCallback(&handle_belief_state);
		      
		      
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
			  DMCS_LOG_DEBUG("Entering round " << next_count);
			  
			  io_service->run(); // wait for one round
			  
			  // model number accounting
			  last_model_count = model_count;
			  model_count = handled_belief_states;
			  diff_count = final_result.size() - diff_count;
			  next_count++;
			  
			  
			  // decide what to do next
			  
			  if (last_round)
			    {
			      keep_running = false;
			    }
			  else
			    {
			      io_service->reset(); // ready for the next round
			      
			      DMCS_LOG_TRACE("diff_count = " << diff_count << ", next_count = " << next_count);
			      
			      if (diff_count > 0 && diff_count < k) // setup next k here
				{
				  DMCS_LOG_TRACE("Got " << diff_count << " partial belief states, getting next batch of size " << k);

				  std::size_t next_k1 = next_count * k + 1;
				  std::size_t next_k2 = next_k1 + k - 1;

				  DMCS_LOG_TRACE("next_k1 = " << next_k1 << ", next_k2 = " << next_k2);
				  
				  mess.setPackRequest(next_k1, next_k2);
				  c.next(mess);
				}
			      else if (pack_size == 0 && model_count > last_model_count)
				{
				  DMCS_LOG_TRACE("fixpoint not reached yet, get next " << k << " belief states");
				  
				  std::size_t next_k1 = next_count * k + 1;
				  std::size_t next_k2 = next_k1 + k - 1;
				  
				  DMCS_LOG_TRACE("next_k1 = " << next_k1 << ", next_k2 = " << next_k2);
				  
				  mess.setPackRequest(next_k1, next_k2);
				  c.next(mess);
				}
			      else // fixpoint reached, last round
				{
				  DMCS_LOG_TRACE("fixpoint reached: model_count = " << model_count << ", final_result: " << final_result.size());
				  
				  c.terminate();
				  last_round = true;
				}
			    }
			}
		      
		      no_beliefstates = final_result.size();
		      
		      //std::cerr << "FINAL RESULT: " << final_result.size() << " belief states." << std::endl;
		      //std::copy(final_result.begin(), final_result.end(), std::ostream_iterator<PartialBeliefState>(std::cerr, "\n"));
		      
		      //delete conflicts;
		      //delete partial_ass;
		      //delete decision;
		      
		      sampler_thread.interrupt();
		      if (sampler_thread.joinable())
			{
			  sampler_thread.join();
			}
		    } // pack_sizee > 0
		}
	      else // opt mode
		{
		  DMCS_LOG_DEBUG("Opt mode.");

		  std::string header = HEADER_REQ_OPT_DMCS;
		  OptCommandType::input_type mess(0); // invoker ID ?
		  Client<OptCommandType> c(*io_service, it, header, mess);

		  DMCS_LOG_DEBUG("Running ioservice.");

		  io_service->run();
	      
		  DMCS_LOG_DEBUG("Getting results.");

		  result = c.getResult();

		  all_answers = true;

#ifdef DMCS_STATS_INFO
		  no_beliefstates = result->getBeliefStates()->size();
#else
		  no_beliefstates = result->size();
#endif
		}
	    }

      // Print results
      // but first read the topology file (quick hack) to get the signatures
      QueryPlanPtr query_plan(new QueryPlan);

#if 0
      query_plan->read_graph(manager);
      BeliefStateListPtr belief_state_list;
#ifdef DMCS_STATS_INFO
      belief_state_list = result->getBeliefStates();
#else
      belief_state_list = result;
#endif

      BeliefStateList::const_iterator bsl_it = belief_state_list->begin();
      BeliefStateList::const_iterator bsl_end = belief_state_list->end();
      --bsl_end;

      for (; bsl_it != belief_state_list->end(); ++bsl_it)
	{
	  BeliefStatePtr belief_state = *bsl_it;

	  BeliefState::const_iterator bs_it = belief_state->begin();
	  BeliefState::const_iterator bs_end = belief_state->end();
	  --bs_end;
	  std::size_t i = 1;

	  std::cout << "(";

	  // Now print only atoms whose corresponding bits are on
	  for (; bs_it != belief_state->end(); ++bs_it, ++i)
	    {
	      std::cout << "{ ";
	      const Signature& sig = query_plan->getSignature(i);
	      const SignatureByLocal& local = boost::get<Tag::Local>(sig);
	      for (std::size_t j = 1; j <= sig.size(); ++j)
		{
		  if (testBeliefSet(*bs_it, j))
		    {
		      SignatureByLocal::const_iterator loc_it = local.find(j);
		      std::cout << loc_it->sym << " ";
		    }
		}
	      if (bs_it != bs_end)
		{
		  std::cout << "}, ";
		}
	      else
		{
		  std::cout << "}";
		}
	    }

	  std::cout << ")" << std::endl;
	  }
#endif//0

#ifdef DEBUG

#ifdef DMCS_STATS_INFO
      std::cerr << "Result: " << std::endl << *result->getBeliefStates() << std::endl;
#else
      std::cerr << "Result: " << std::endl << *result << std::endl;
#endif // DMCS_STATS_INFO
      
#endif // DEBUG
      
      
#ifdef DMCS_STATS_INFO
      std::cout << "# " << no_beliefstates << std::endl;
      std::cout << *result->getStatsInfo() << std::endl;
#else
      std::cout << "Total Number of Equilibria: " << no_beliefstates;

      if (!complete)
	{
	  std::cout << "+";
	}

      std::cout << std::endl;
#endif // DMCS_STATS_INFO
	}
    }
  catch (std::exception& e)
    {
      DMCS_LOG_FATAL("Bailing out: " << e.what());
      return 1;
    }
  catch (...)
    {
      DMCS_LOG_FATAL("Exception of unknown type.");
      return 1;
    }
  
  return 0;
}

// Local Variables:
// mode: C++
// End:
