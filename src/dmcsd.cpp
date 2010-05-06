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
 * @file   dmcsd.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Fri Jan  8 17:16:59 2010
 * 
 * @brief  
 * 
 * 
 */


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "Server.h"
#include "Message.h"
#include "DimacsVisitor.h"
#include "Session.h"
#include "LocalKBBuilder.h"
#include "PropositionalASPGrammar.h"
#include "BRGrammar.h"
#include "BridgeRulesBuilder.h"
#include "ParserDirector.h"
#include "Signature.h"
#include "LocalLoopFormulaBuilder.h"
#include "CNFLocalLoopFormulaBuilder.h"
#include "LoopFormulaDirector.h"
#include "PrimitiveDMCS.h"
#include "OptDMCS.h"
#include "CommandType.h"
#include "ProgramOptions.h"
#include "Neighbor.h"

#include <string>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>

//typedef boost::shared_ptr<Server> ServerPtr;

using namespace dmcs;

const char* TOP_EXT = ".top";
const char* OPT_EXT = ".opt";

int main(int argc, char* argv[])
{
  try
    {
      boost::program_options::options_description desc("Allowed options");
      desc.add_options()
	(HELP, "produce help and usage message")
	(CONTEXT_ID, boost::program_options::value<std::size_t>(), "set context ID")
	(PORT, boost::program_options::value<int>()->default_value(DEFAULT_PORT), "set port")
	(KB, boost::program_options::value<std::string>(), "set Knowledge Base file name")
	(BR, boost::program_options::value<std::string>(), "set Bridge Rules file name")
	(MANAGER, boost::program_options::value<std::string>(), "set Manager HOST:PORT")
	(TOPOLOGY, boost::program_options::value<std::string>(), "set Topology file name")
	;
      
      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    
      
      if (vm.count(HELP)) {
	
	std::cerr << "Usage: " << argv[0] << " --" << CONTEXT_ID << "=ID [--" << PORT << "=PORT] --" << KB << "=LOCAL_KB(FILE) --" << BR << "=BRIDGE_RULES(FILE) [--" << MANAGER << "=HOSTNAME:PORT|--" << TOPOLOGY << "=TOPOLOGY(FILE)]" << std::endl;
	return 1;
      }
      
      int myport = 0;
      std::size_t myid = 0;	
      std::string filename_local_kb = "";
      std::string filename_bridge_rules = "";
      std::string filename_topo = "";
      
      if (vm.count(CONTEXT_ID)) 
	{
	  myid = vm[CONTEXT_ID].as<std::size_t>();
	}

      if (vm.count(PORT)) 
	{
	  myport = vm[PORT].as<int>();
	}

      if (vm.count(KB)) 
	{
	  filename_local_kb = vm[KB].as<std::string>();
	}

      if (vm.count(BR)) 
	{
	  filename_bridge_rules = vm[BR].as<std::string>();
	}

	int optionalCount = 0;

	if (vm.count(MANAGER)) 
	  {
	    optionalCount++;
	    std::cerr << "We are sorry, but the manager feature is under implementation, please try the other alternatives";
	    //read manager host and port
	    return 1;
	  }

	if (vm.count(TOPOLOGY)) 
	  {
	    filename_topo = vm[TOPOLOGY].as<std::string>();
	    optionalCount++;
	  }

	if(myid == 0 || filename_local_kb.compare("") == 0 || filename_bridge_rules.compare("") == 0|| optionalCount == 0 || optionalCount == 2) 
	  {
	    std::cout << desc << "\n";
	    return 1;
	  }
      // if (argc != 7)
      // 	{
      // 	  std::cerr << "Usage: " << argv[0] << " ID HOST PORT LOCAL_KB(FN) BRIDGE_RULES(FN) TOPOLOGY(FN)";
      // 	  return 1;
      // 	}
      
      // std::size_t myid = std::atoi(argv[1]);
      // std::string hostname = argv[2];
      // int myport = std::atoi(argv[3]);
      // std::string filename_local_kb = argv[4];
      // std::string filename_bridge_rules = argv[5];
      // std::string filename_topo = argv[6];

      //      SignaturePtr sig(new Signature);

 #ifdef DEBUG
	std::cout << "we have read everything and we will start" << std::endl;
	std::cout << "myid: " << myid <<std::endl;
	std::cout << "local KB: " << filename_local_kb <<std::endl;
	std::cout << "Bridge Rules: " << filename_bridge_rules <<std::endl;
	std::cout << "Topology: " << filename_topo <<std::endl;
 #endif 
	///@todo change when the manager is added
      QueryPlanPtr query_plan(new QueryPlan);


      query_plan->read_graph(filename_topo);
      std::size_t system_size = query_plan->getSystemSize();
      //NeighborListPtr neighbor_list = query_plan->getNeighbors(myid);

      // get the global signature from the query plan. Later we might
      // find another way to pass it to dmcsd, so that we don't have
      // to depend on the query plan, which is just for the purpose of
      // opt-dmcs and should be computed from neighbors + global_sigs,
      // by the manager.

      ///@todo: have config file to provide Signature
      SignatureVecPtr global_sigs(new SignatureVec);
      for (std::size_t i = 1; i <= system_size; ++ i)
	{
	  SignaturePtr s(new Signature);
	  *s = query_plan->getSignature(i);
	  global_sigs->push_back(s);
	}

      // get my local signature
      SignaturePtr sig = (*global_sigs)[myid-1];

// #ifdef DEBUG
//       std::cerr << "Server information              " << std::endl;
//       std::cerr << "Id:                             " << myid << std::endl;
//       std::cerr << "Port:                           " << myport << std::endl;
//       std::cerr << "Filename for local KB:          " << filename_local_kb << std::endl;
//       std::cerr << "Filename for bridge rules:      " << filename_bridge_rules << std::endl;
//       std::cerr << "Filename for topology:          " << filename_original_topo << std::endl;
//       std::cerr << "My signature:                   " << *sig << std::endl;
// #endif
      
      //TheoryPtr theory(new Theory);
      RulesPtr local_kb(new Rules);
      BridgeRulesPtr bridge_rules(new BridgeRules);
      //BridgeAtomSetPtr br(new BridgeAtomSet);

      LocalKBBuilder<PropositionalASPGrammar> builder1(local_kb, sig);
      ParserDirector<PropositionalASPGrammar> parser_director;
      parser_director.setBuilder(&builder1);
      parser_director.parse(filename_local_kb);

#ifdef DEBUG
      for (Signature::const_iterator i = sig->begin(); i != sig->end(); ++i)
	{
	  std::cerr << *i << std::endl;
	}
#endif

      // now we got the local KB,
      // going to parse the bridge rules
      NeighborListPtr neighbor_list(new NeighborList);

      BridgeRulesBuilder<BRGrammar> builder_br(myid, bridge_rules, neighbor_list, global_sigs);
      ParserDirector<BRGrammar> parser_director_br;
      parser_director_br.setBuilder(&builder_br);
      parser_director_br.parse(filename_bridge_rules);

      std::cerr << "Finished parsing bridge rules" << std::endl;

      for (NeighborList::const_iterator it = neighbor_list->begin(); it != neighbor_list->end(); ++it)
	{
	  NeighborPtr nb = *it;
	  nb->hostname = query_plan->getHostname(nb->neighbor_id);
	  nb->port     = query_plan->getPort(nb->neighbor_id);
	}

#ifdef DEBUG
      std::cerr << "My neighbors: " << *neighbor_list << std::endl;
#endif // DEBUG

      // setup my context
      ContextPtr ctx(new Context(myid, system_size, sig, local_kb, bridge_rules, neighbor_list));

      boost::asio::io_service io_service;
      boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), myport);    
 
      //compute size local signature
      const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);
      
      SignatureByCtx::const_iterator low = local_sig.lower_bound(myid);
      SignatureByCtx::const_iterator up  = local_sig.upper_bound(myid);
      
#ifdef DEBUG
      std::cerr << myid << std::endl;
#endif
      
      std::size_t size = std::distance(low, up);

#ifdef DEBUG
      std::cerr << "Sig input to LF" << *sig <<std::endl;
#endif

      //construct loop formulae
      CNFLocalLoopFormulaBuilder lf_builder(sig, size);
      LoopFormulaDirector director;
      director.setBuilder(&lf_builder);
      director.construct(local_kb, bridge_rules);
      
      TheoryPtr loopFormula;
      loopFormula = lf_builder.getFormula();

#ifdef DEBUG      
      DimacsVisitor v(std::cerr);
      v.visitTheory(loopFormula, sig->size());
#endif
      
     
      // this result Sig will only be different in case of using an EquiCNF builder
      //      SignaturePtr resultSig;
      //      resultSig = lf_builder.getSignature();

      DMCSPtr dmcs;
      boost::shared_ptr<BaseServer> server;

      // setup the server
      if (filename_topo.find(TOP_EXT) != std::string::npos)
	{
	  PrimitiveDMCSPtr d(new PrimitiveDMCS(ctx, loopFormula, global_sigs));
	  PrimitiveCommandType pdmcs(d);
	  
	  boost::shared_ptr<BaseServer> s(new Server<PrimitiveCommandType>(pdmcs, io_service, endpoint));      
	  server = s;
	}
      else if (filename_topo.find(OPT_EXT) != std::string::npos)
	{
	  OptDMCSPtr d(new OptDMCS(ctx, loopFormula, global_sigs, query_plan));
	  OptCommandType odmcs(d);

	  boost::shared_ptr<BaseServer> s(new Server<OptCommandType>(odmcs, io_service, endpoint));  
	  server = s;
	}
      else 
	{
	  std::cerr << "Topology not recognised!" << std::endl;
	  return 1;
	}

      
      boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
      io_service.run();
      t.join();
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
  
  return 0;
}
