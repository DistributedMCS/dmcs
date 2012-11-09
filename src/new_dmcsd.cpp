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
 * @file   new_dmcsd.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Oct  12 15:10:26 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/program_options.hpp>

#include "dmcs/ProgramOptions.h"
#include "dmcs/DLVEngine.h"
#include "dmcs/DLVInstantiator.h"
#include "dmcs/DLVEvaluator.h"
#include "mcs/BeliefStateOffset.h"
#include "mcs/BeliefTable.h"
#include "mcs/ID.h"
#include "mcs/NewContext.h"
#include "mcs/QueryPlan.h"
#include "mcs/RequestDispatcher.h"
#include "network/NewClient.h"
#include "network/NewConcurrentMessageDispatcher.h"
#include "network/NewServer.h"
#include "parser/BridgeRuleParser.h"
#include "parser/QueryPlanParser.h"
#include "parser/ReturnPlanParser.h"

using namespace dmcs;

int
main(int argc, char* argv[])
{
  try
    {
      int myport = 0;
      std::size_t myid = 0;
      std::size_t system_size = 0;
      std::size_t queue_size = 0;
      std::size_t bs_size = 0; // for testing, the generator should take care of this value
      std::size_t pack_size = 0;
      std::string manager;
      std::string filename_local_kb;
      std::string filename_bridge_rules;
      std::string filename_query_plan;
      std::string filename_return_plan;

      const char* help_description = "\n\
Usage: dmcsd --context=N --port=PORT --kb=FILE --br=FILE --queryplan=FILE [OPTIONS]\n\n\
Options";

      boost::program_options::options_description desc(help_description);
      desc.add_options()
	(HELP, "produce help and usage message")
	(CONTEXT_ID, boost::program_options::value<std::size_t>(&myid), "set context ID")
	(PORT, boost::program_options::value<int>(&myport), "set port")
	(MANAGER, boost::program_options::value<std::string>(&manager), "set Manager HOST:PORT")
	(SYSTEM_SIZE, boost::program_options::value<std::size_t>(&system_size), "set system size")
	(QUEUE_SIZE, boost::program_options::value<std::size_t>(&queue_size)->default_value(DEFAULT_QUEUE_SIZE), "set concurrent message queue size")
	(BS_SIZE, boost::program_options::value<std::size_t>(&bs_size), "set belief state size")
	(PACK_SIZE, boost::program_options::value<std::size_t>(&pack_size)->default_value(DEFAULT_PACK_SIZE), "set size of packages of belief states to be transferred")
	(KB, boost::program_options::value<std::string>(&filename_local_kb), "set Knowledge Base file name")
	(BR, boost::program_options::value<std::string>(&filename_bridge_rules), "set Bridge Rules file name")
	(QP, boost::program_options::value<std::string>(&filename_query_plan), "set Query Plan file name")
	(RP, boost::program_options::value<std::string>(&filename_return_plan), "set Return Plan file name")
	;

      boost::program_options::variables_map vm;
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);

      if (vm.count(HELP))
	{
	  std::cerr << desc << std::endl;
	  return 1;
	}

      if (myport == 0 || system_size == 0 || bs_size == 0 ||
	  filename_local_kb.empty() || filename_bridge_rules.empty() || 
	  filename_query_plan.empty() || filename_return_plan.empty() ||
	  manager.empty())
	{
	  std::cerr << "The following options are mandatory: --port, --manager " << std::endl 
	            << "                                     --system-size, --belief-state-size," << std::endl
		    << "                                     --kb, --br, --queryplan, --returnplan." << std::endl;
	  std::cerr << desc << std::endl;
	  return 1;
	}

      BeliefStateOffset* bso_instance = BeliefStateOffset::create(system_size, bs_size);

      // extract manager's hostname and port
      std::size_t split_colon = manager.find_last_of(":");
      if (split_colon == std::string::npos)
	{
	  std::cerr << "--manager must be of the form HOST:PORT" << std::endl;
	  std::cerr << desc << std::endl;
	  return 1;
	}
      std::string manager_hostname = manager.substr(0, split_colon);
      std::string manager_port = manager.substr(split_colon+1);

      ReturnPlanMapPtr return_plan = ReturnPlanParser::parseFile(filename_return_plan);

      ContextQueryPlanMapPtr queryplan_map = QueryPlanParser::parseFile(filename_query_plan);
      const ContextQueryPlan& local_queryplan = queryplan_map->find(myid)->second;
      BridgeRuleParserReturnVal ret_val = BridgeRuleParser::parseFile(filename_bridge_rules, queryplan_map, myid);;
      BridgeRuleTablePtr bridge_rules = ret_val.first;
      NewNeighborVecPtr neighbors = ret_val.second;

      EnginePtr dlv_engine = DLVEngine::create();
      EngineWPtr dlv_engine_wp(dlv_engine);
      InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, filename_local_kb);

      NewContextVecPtr ctx_vec(new NewContextVec);
 
      NewContextPtr ctx(new NewContext(myid, pack_size, dlv_inst, 
				       local_queryplan.localSignature, 
				       return_plan, bridge_rules, neighbors));
      ctx_vec->push_back(ctx);
     
      if (!neighbors->empty())
	{
	  std::cerr << "Intermediate context" << std::endl;
	  for (NewNeighborVec::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
	    {
	      std::cout << **it << std::endl;
	    }
	}

      RegistryPtr reg(new Registry(system_size, queue_size, bs_size, 
				   manager_hostname, manager_port, ctx_vec));
      
      boost::asio::io_service io_service_server;
      boost::asio::ip::tcp::endpoint endpoint_server(boost::asio::ip::tcp::v4(), myport);

      NewServer s(io_service_server, endpoint_server, reg);
      
      boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service_server));
      io_service_server.run();
      t.join();
    }
  catch (std::exception& e)
    {
      std::cerr << "Bailing out: " << e.what() << std::endl;
      return 1;
    }
  return 0;
}

// try me:
// ./new_dmcsd --context=1 --port=5001 --system-size=4 --belief-state-size=20 --kb=../../examples/context1.lp --br=../../examples/context1.br --queryplan=../../examples/context1.qp


// Local Variables:
// mode: C++
// End:
