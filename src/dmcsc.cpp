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

#include "dmcs/OptCommandType.h"
#include "dmcs/PrimitiveCommandType.h"
#include "dmcs/StreamingCommandType.h"

#include "dyndmcs/DynamicCommandType.h"
#include "dyndmcs/DynamicConfiguration.h"
#include "dyndmcs/InstantiatorCommandType.h"

#include "network/Client.h"

#include "dmcs/Message.h"
#include "dmcs/QueryPlan.h"
#include "dmcs/ProgramOptions.h"

#include "mcs/Theory.h"
#include "mcs/BeliefState.h"

#include <fstream>
#include <iostream>
#include <string> 

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/program_options.hpp>


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
      std::cerr << "Instantiation finished successfully!" << std::endl;

      // Now call the evaluation
    }
}

int
main(int argc, char* argv[])
{
  try 
    {
      std::string hostName = "";
      std::string port = "";
      std::string manager = "";
      std::string qvs = "";
      std::size_t system_size = 0;
      std::size_t root_ctx;
      bool dynamic;
      BeliefStatePtr V(new BeliefState);

      boost::program_options::options_description desc("Allowed options");

      desc.add_options()
	(HELP, "produce help and usage message")
	(HOSTNAME, boost::program_options::value<std::string>(&hostName)->default_value("localhost"), "set host name")
	(PORT, boost::program_options::value<std::string>(&port), "set port")
	(QUERY_VARS, boost::program_options::value<std::string>(&qvs), "set query variables")
	(SYSTEM_SIZE, boost::program_options::value<std::size_t>(&system_size), "set system size")
	(MANAGER, boost::program_options::value<std::string>(&manager), "set Manager HOST:PORT")
	(DYNAMIC, boost::program_options::value<bool>(&dynamic)->default_value(false), "set to dynamic mode")
	(ROOT_CTX, boost::program_options::value<std::size_t>(&root_ctx)->default_value(1), "set root context id")
	;
	
      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    

      if (vm.count(HELP))
	{
	  std::cerr << desc << std::endl;
	  return 1;
        }
      
      bool primitiveDMCS = false;
      if (qvs.compare("") != 0) // reading V for basic DMCS
	{
	  primitiveDMCS = true;
	  std::istringstream iss(qvs);
	  iss >> V;

	  
	  //	  boost::tokenizer<> tok(qvs);



	  /*	  for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it)
	    {
	      std::istringstream iss(it->c_str());
	      BeliefSet bs;
	      iss >> bs;
	      V->push_back(bs);
	    }
	  */
	}

      if (port.compare("") == 0)
	{
	  std::cerr << desc << "\n";
	  return 1;
	}

      boost::asio::io_service io_service;
      boost::asio::ip::tcp::resolver resolver(io_service);
      boost::asio::ip::tcp::resolver::query query(hostName, port);
      boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
      boost::asio::ip::tcp::endpoint endpoint = *it;

      if (dynamic)
	{ // dynamic mode

	  std::cerr << "In dynamic mode"<< std::endl;

	  ContextSubstitutionPtr ctx_sub(new ContextSubstitution);
	  ConfigMessage mess(root_ctx, ctx_sub, false);
	  
#ifdef DEBUG
	  std::cerr << "Message = " << mess << std::endl;
#endif
	  
	  std::string header = HEADER_REQ_DYN_DMCS;
	  Client<DynamicCommandType> c(io_service, it, header, mess);
	  io_service.run();

	  DynamicConfiguration::dynmcs_return_type result = c.getResult();

	  std::cerr << "FINAL RESULT: " << std::endl
		    << *result << std::endl;


	  // instantiate the system, then write .br and .sh files
	  /*for (ContextSubstitutionList::const_iterator it = result->begin(); it != result->end(); ++it)
	    {
	      instantiate(*it);
	      }*/
	  ContextSubstitutionList::const_iterator it = result->begin();
	  instantiate(*it, hostName, port);
	}
      else
	{ // ground mode
	  if(system_size == 0 ||
	     (primitiveDMCS && V->size() == 0) ||
	     (primitiveDMCS && V->size() != system_size))
	    {
	      std::cerr << desc << "\n";
	      return 1;
	    }

	  // our result
#ifdef DMCS_STATS_INFO
	  ReturnMessagePtr result(new ReturnMessage);
#else
	  BeliefStateListPtr result(new BeliefStateList);
#endif // DMCS_STATS_INFO
	  
#ifdef DEBUG
	  std::cerr << "Starting DMCS with " << system_size << std::endl;
#endif
	  
	  if (primitiveDMCS) // primitive DMCS
	    {
#ifdef DEBUG
	      std::cerr << "Primitive" << std::endl;
	      std::cerr << "Going to send: ";
	      std::cerr << V << std::endl;
#endif 
	      std::string header = HEADER_REQ_PRI_DMCS;
	      PrimitiveMessage mess(V);
	      Client<PrimitiveCommandType> c(io_service, it, header, mess);
	      
#ifdef DEBUG
	      std::cerr << "Running ioservice" <<std::endl;
#endif
	  
	      io_service.run();

#ifdef DEBUG	  
	      std::cerr << "Getting results" <<std::endl;
#endif
	  
	      result = c.getResult();
	    }
	  else // Opt DMCS
	    {
	      std::string header = HEADER_REQ_OPT_DMCS;
	      OptCommandType::input_type mess(0); // invoker ID ?
	      Client<OptCommandType> c(io_service, it, header, mess);
	      io_service.run();
	      
	      result = c.getResult();
	    }

      // Print results
      // but first read the topology file (quick hack) to get the signatures
      QueryPlanPtr query_plan(new QueryPlan);

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

      /*
      std::cerr << "Signatures: " << std::endl << sigs << std::endl;*/


#ifdef DEBUG

#ifdef DMCS_STATS_INFO
      std::cerr << "Result: " << std::endl << *result->getBeliefStates() << std::endl;
#else
      std::cerr << "Result: " << std::endl << *result << std::endl;
#endif // DMCS_STATS_INFO
      
#endif // DEBUG
      

#ifdef DMCS_STATS_INFO
      std::cout << "# " << result->getBeliefStates()->size() << std::endl;
      std::cout << *result->getStatsInfo() << std::endl;
#else
      std::cout << "Total Number of Equilibria: " << result->size() << std::endl;
#endif
	}
    }
  catch (std::exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }
  catch (...)
    {
      std::cerr << "Exception of unknown type!" << std::endl;
    }
  
  return 0;
}

// Local Variables:
// mode: C++
// End:
