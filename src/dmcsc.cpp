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

#include "Client.h"
#include "Theory.h"
#include "BeliefState.h"
#include "Message.h"
#include "QueryPlan.h"
#include "ProgramOptions.h"

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


int
main(int argc, char* argv[])
{
  try 
    {
      std::string hostName = "";
      std::string port = "";
      std::size_t systemSize = 0;
      std::size_t root_ctx;
      bool dynamic;
      BeliefStatePtr V(new BeliefState);

      boost::program_options::options_description desc("Allowed options");

      desc.add_options()
	(HELP, "produce help and usage message")
	(HOSTNAME, boost::program_options::value<std::string>(&hostName), "set host name")
	(PORT, boost::program_options::value<std::string>(&port), "set port")
	(QUERY_VARS, boost::program_options::value<std::string>(), "set query variables")
	(SYSTEM_SIZE, boost::program_options::value<std::size_t>(), "set system size")
	(MANAGER, boost::program_options::value<std::string>(), "set Manager HOST:PORT")
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
      if (vm.count(QUERY_VARS)) // reading V for basic DMCS
	{
	  primitiveDMCS = true;
	  
	  const std::string& qvs = vm[QUERY_VARS].as<std::string>();
	    
	  boost::tokenizer<> tok(qvs);

	  for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it)
	    {
	      std::istringstream iss(it->c_str());
	      BeliefSet bs;
	      iss >> bs;
	      V->push_back(bs);
	    }
	}

	  for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it)
	    {
	      std::istringstream iss(it->c_str());
	      BeliefSet bs;
	      iss >> bs;
	      V->push_back(bs);
	    }

      int optionalCount = 0;

      if (vm.count(MANAGER)) 
	{
	  //optionalCount++;
	  //std::cerr << "We are sorry, but the manager feature is under implementation, please try the other alternatives";
	  //read manager host and port
	  // quick hack: manager now stands for the topology file. 
	  manager = vm[MANAGER].as<std::string>();
	  
	  //return 1;
	}

      if (vm.count(SYSTEM_SIZE)) 
	{
	  systemSize = vm[SYSTEM_SIZE].as<std::size_t>();
	  optionalCount++;
	}

      boost::asio::io_service io_service;
      boost::asio::ip::tcp::resolver resolver(io_service);
      boost::asio::ip::tcp::resolver::query query(hostName, port);
      boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
      boost::asio::ip::tcp::endpoint endpoint = *it;

      if (dynamic)
	{
	  // dynamic mode

	  if(hostName.compare("") ==0 ||
	     port.compare("") == 0)
	    {
	      std::cerr << desc << "\n";
	      return 1;
	    }


	  ContextSubstitutionPtr ctx_sub(new ContextSubstitution);
	  ConfigMessage mess(root_ctx, ctx_sub, false);
	  
#ifdef DEBUG
	  std::cerr << "Message = " << mess << std::endl;
#endif
	  
	  Client<DynamicCommandType> c(io_service, it, mess);
	  io_service.run();
	}
      else
	{
	  // ground mode

	  if(hostName.compare("") ==0 ||
	     port.compare("") == 0 ||
	     systemSize == 0 ||
	     (primitiveDMCS && V->size() == 0) ||
	     (primitiveDMCS && V->size() != systemSize) ||
	     optionalCount == 0 || optionalCount == 2) 
	    {
	      std::cerr << desc << "\n";
	      return 1;
	    }


	  // our result
	  BeliefStateListPtr belief_states(new BeliefStateList);
	
#ifdef DEBUG
	  std::cerr << "Starting the DMCS with " << systemSize << std::endl;
#endif
      
	  if (primitiveDMCS) //primitive DMCS
	    {
#ifdef DEBUG
	      std::cerr << "Primitive" << std::endl;
	      std::cerr << "Going to send: ";
	      std::cerr << V << std::endl;
#endif 
	  
	      PrimitiveMessage mess(V);
	      Client<PrimitiveCommandType> c(io_service, it, mess);

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
	      OptCommandType::input_type mess(0); // invoker ID ?
	      Client<OptCommandType> c(io_service, it, mess);
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
