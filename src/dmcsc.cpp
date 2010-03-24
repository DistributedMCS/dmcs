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


//const char* TOP_EXT = ".top";
//const char* OPT_EXT = ".opt";

BeliefStatePtr V;


int main(int argc, char* argv[])
{
    try {

        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
	  ("help", "produce help and usage message")
	  ("hostname,h", boost::program_options::value<std::string>(), "set host name")
	  ("port,p", boost::program_options::value<std::string>(), "set port")
	  ("query-variables,v", boost::program_options::value<std::string>(), "set port")
	  ("systemsize,s", boost::program_options::value<std::size_t>(), "set system size")
	  ("manager,m", boost::program_options::value<std::string>(), "set manager HOST:PORT")
	  ;
	
        boost::program_options::variables_map vm;        
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);    

        if (vm.count("help")) {

	  std::cerr << "Usage: " << argv[0] << " --hostname=<HOST> --port=<PORT> --query-variables=V [--manager=HOSTNAME:PORT|--systemsize=<SIZE>]" << std::endl;
	  std::cerr << "Or: " << argv[0] << " -h=<HOST> -p=<PORT> -v=V [-m=HOSTNAME:PORT|-s=<SIZE>]" << std::endl;
	  return 1;
        }

	std::string hostName = "";
	std::string port = "";
	std::size_t systemSize = 0;	

	if (vm.count("hostname")) 
	  {
	    hostName = vm["hostname"].as<std::string>();
	  }

	if (vm.count("port")) 
	  {
	    port = vm["port"].as<std::string>();
	  }

	bool primitiveDMCS = false;
	if (vm.count("query-variables")) // reading the V
	  {
	    primitiveDMCS = true;
	    boost::tokenizer<> s(vm["query-variables"].as<std::string>());
	    for (boost::tokenizer<>::iterator it = s.begin(); it != s.end(); ++it)
	      {
		V.belief_state_ptr->belief_state.push_back(std::atoi(it->c_str()));
	      }
	  }

	int optionalCount = 0;

	if (vm.count("manager")) 
	  {
	    optionalCount++;
	    //read manager host and port
	  }

	if (vm.count("systemsize")) 
	  {
	    systemSize = vm["systemsize"].as<std::size_t>();
	    optionalCount++;
	  }

	if(hostName.compare("") ==0 || port.compare("") == 0 || optionalCount == 0 || optionalCount == 2) 
	  {
	    std::cout << desc << "\n";
	    return 1;
	  }

	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(io_service);
	boost::asio::ip::tcp::resolver::query query(hostName, port);
	boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
	boost::asio::ip::tcp::endpoint endpoint = *iterator;
      
//       std::string filename_topo = argv[3];
//       QueryPlanPtr query_plan(new QueryPlan);
     
//       query_plan->read_graph(filename_topo);

//       std::size_t systemSize = query_plan->getSystemSize();
	BeliefStatesPtr belief_states;
	
#ifdef DEBUG
	std::cerr << "Starting the DMCS with " << systemSize << std::endl;
#endif
	
	if (primitiveDMCS) //primitive DMCS
	  {
	    
#ifdef DEBUG
	    std::cerr << "Primitive" <<std::endl;
#endif
	    
	    // 	  V = query_plan->getGlobalV();
	    PrimitiveMessage mess(V);
	    Client<PrimitiveMessage> c(io_service, 
				       iterator,
				       systemSize,
				       mess);

#ifdef DEBUG
	    std::cerr << "Running ioservice" <<std::endl;
#endif
	    
	    io_service.run();
#ifdef DEBUG
	    std::cerr << "Getting BeliefStates" <<std::endl;
#endif
	    belief_states= c.getBeliefStates();
 	}
       else // Opt DMCS
 	{
	  
 	  OptMessage mess(0); // invoker ID ?
 	  Client<OptMessage> c(io_service, 
 			       iterator,
 			       systemSize,
 			       mess);
 	  io_service.run();
 	  belief_states= c.getBeliefStates();
 	}

 #ifdef DEBUG
       std::cerr << "Going to send: ";
       std::cerr << V << std::endl;
 #endif 

       //decode_result(belief_states);

       //      std::cerr << "Result: " << std::endl << belief_states << std::endl;
       std::cout << "Number of answers: " << belief_states.belief_states_ptr->belief_states.size() << std::endl;
       //std::cerr << "Result: " << std::endl << belief_states << std::endl;


    }
    catch(std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
      std::cerr << "Exception of unknown type!\n";
    }


//   try
//     {
//       if (argc != 4)
// 	{
// 	  std::cerr << "Usage: " << argv[0] << "<HOST> <PORT> <TOPO_FILENAME>" << std::endl;
// 	  return 1;
// 	}

// #ifdef DEBUG
//       std::cerr << "Calling to:        " << std::endl;
//       std::cerr << "Host:              " << argv[1] << std::endl;
//       std::cerr << "Port:              " << argv[2] << std::endl;
//       std::cerr << "Topology filename: " << argv[3] << std::endl;
// #endif

//       ///@todo: create appropriate mess, also need to change client

//       boost::asio::io_service io_service;
//       boost::asio::ip::tcp::resolver resolver(io_service);
//       boost::asio::ip::tcp::resolver::query query(argv[1], argv[2]);
//       boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
//       boost::asio::ip::tcp::endpoint endpoint = *iterator;
      
//       std::string filename_topo = argv[3];
//       QueryPlanPtr query_plan(new QueryPlan);
     
//       query_plan->read_graph(filename_topo);

//       std::size_t systemSize = query_plan->getSystemSize();
//       wBeliefStatesPtr belief_states;

// #ifdef DEBUG
//       std::cerr << "Starting the DMCS with " << systemSize << std::endl;
// #endif

//       if (filename_topo.find(TOP_EXT) != std::string::npos) //primitive DMCS
// 	{

// #ifdef DEBUG
// 	  std::cerr << "Primitive" <<std::endl;
// #endif
	  
// 	  V = query_plan->getGlobalV();
// 	  PrimitiveMessage mess(V);
// 	  Client<PrimitiveMessage> c(io_service, 
// 				     iterator,
// 				     systemSize,
// 				     mess);

// #ifdef DEBUG
// 	  std::cerr << "Running ioservice" <<std::endl;
// #endif

// 	  io_service.run();
// #ifdef DEBUG
// 	  std::cerr << "Getting BeliefStates" <<std::endl;
// #endif
// 	  belief_states= c.getBeliefStates();
// 	}
//       else if (filename_topo.find(OPT_EXT) != std::string::npos) // Opt DMCS
// 	{
// 	  ///@todo get system size from somewhere to pass to client
// 	  OptMessage mess(0); // invoker ID ?
// 	  Client<OptMessage> c(io_service, 
// 			       iterator,
// 			       systemSize,
// 			       mess);
// 	  io_service.run();
// 	  belief_states= c.getBeliefStates();
// 	}

// #ifdef DEBUG
//       std::cerr << "Going to send: ";
//       std::cerr << V << std::endl;
// #endif 

//       //decode_result(belief_states);

//       //      std::cerr << "Result: " << std::endl << belief_states << std::endl;
//       std::cout << "Number of answers: " << belief_states.belief_states_ptr->belief_states.size() << std::endl;
//       //std::cerr << "Result: " << std::endl << belief_states << std::endl;


// #ifdef DEBUG
//       std::cerr << "Result: " << std::endl << belief_states << std::endl;
//       print_result_file(belief_states, filename);
// #endif
//     }
//   catch (std::exception& e)
//     {
//       std::cerr << "Exception: " << e.what() << std::endl;
//     }
  
  return 0;
}
