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
 * @file   staticGen.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Aug  30 13:55:00 2010
 * 
 * @brief  
 * 
 * 
 */

#include "generator/dmcsGen.h"
#include "generator/BinaryTreeTopoGenerator.h"
#include "generator/DiamondTopoGenerator.h"
#include "generator/DiamondOptTopoGenerator.h"
#include "generator/DiamondArbitraryTopoGenerator.h"
#include "generator/DiamondZigZagTopoGenerator.h"
#include "generator/DiamondZigZagOptTopoGenerator.h"
#include "generator/HouseTopoGenerator.h"
#include "generator/HouseOptTopoGenerator.h"
#include "generator/MultipleRingTopoGenerator.h"
#include "generator/MultipleRingOptTopoGenerator.h"
#include "generator/RingTopoGenerator.h"
#include "generator/RingOptTopoGenerator.h"
#include "generator/RingEdgeTopoGenerator.h"
#include "generator/ContextGenerator.h"
#include "generator/QueryPlanWriter.h"

#include "dmcs/ProgramOptions.h"
#include "dmcs/QueryPlan.h"
#include "dmcs/Log.h"

#include "mcs/Rule.h"
#include "mcs/Signature.h"

#include <boost/program_options.hpp>

#include <sys/time.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace dmcs;
using namespace dmcs::generator;

#define DMCSD "dmcsd"
#define DMCSC "dmcsc"
#define TESTSDIR "tests"
#define STR_LOCALHOST "localhost"
#define LP_EXT  ".lp"
#define BR_EXT  ".br"
#define TOP_EXT ".top"
#define OPT_EXT ".opt"
#define ANS_EXT ".ans"
#define CMD_EXT "_command_line.txt"
#define SH_CMD_EXT "_command_line.sh"
#define OPT_CMD_EXT "_command_line_opt.txt"
#define OPT_SH_CMD_EXT "_command_line_opt.sh"
#define DLV_EXT ".dlv"
#define DLV_CMD_EXT "_dlv.txt"
#define OPT_DLV_CMD_EXT "_dlv_opt.txt"

#define BASE_PORT 5000

#define HELP_MESSAGE_TOPO "Available topologies:\n\
   0: Random Topology\n\
   1: Chain of diamonds Topology (all ways down)\n\
   2: Chain of diamonds Topology (arbitrary edges)\n\
   3: Chain of zig-zag diamonds Topology\n\
   4: Pure Ring Topology\n\
   5: Ring Topology (with additional edges)\n\
   6: Binary Tree Topology\n\
   7: House Topology\n\
   8: Multiple Ring Topology"


SignatureVecPtr sigmas(new SignatureVec);
InterfaceVecPtr context_interfaces(new InterfaceVec);
BeliefStatePtr  minV;
LocalInterfaceMapPtr lcim(new LocalInterfaceMap);
LocalInterfaceMapPtr opt_lcim(new LocalInterfaceMap);

QueryPlanPtr orig_qp(new QueryPlan);
QueryPlanPtr opt_qp(new QueryPlan);

TopologyGenerator* orig_topo_gen;
OptTopologyGenerator* opt_topo_gen;

NeighborVec2Ptr orig_topo(new NeighborVec2);
NeighborVec2Ptr opt_topo(new NeighborVec2);

std::size_t no_contexts;
std::size_t no_atoms;
std::size_t no_interface_atoms;
std::size_t no_bridge_rules;
std::size_t topology_type;

std::string prefix;
std::string filename;
std::string logging;

std::ofstream file_rules;
std::ofstream file_topo;
std::ofstream file_command_line;
std::ofstream file_command_line_sh;
std::ofstream file_command_line_opt;
std::ofstream file_command_line_opt_sh;
std::ofstream file_dlv;
std::ofstream file_command_line_dlv;
std::ofstream file_command_line_dlv_opt;

int 
read_input(int argc, char* argv[])
{
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    (HELP, "Help message")
    (CONTEXTS, boost::program_options::value<std::size_t>(&no_contexts)->default_value(7), "Number of contexts")
    (ATOMS, boost::program_options::value<std::size_t>(&no_atoms)->default_value(8), "Number of contexts")
    (INTERFACE, boost::program_options::value<std::size_t>(&no_interface_atoms)->default_value(4), "Number of interface atoms")
    (BRIDGE_RULES, boost::program_options::value<std::size_t>(&no_bridge_rules)->default_value(4), "Number of interface atoms")
    (TOPOLOGY, boost::program_options::value<std::size_t>(&topology_type)->default_value(1), HELP_MESSAGE_TOPO)
    (PREFIX, boost::program_options::value<std::string>(&prefix)->default_value("student"), "Prefix for all files")
    (LOGGING, boost::program_options::value<std::string>(&logging)->default_value(""), "log4cxx config file")
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count(HELP) || prefix.compare("") == 0 || no_contexts == 0 || no_atoms == 0 ||
       no_interface_atoms == 0 || no_bridge_rules == 0)
    {
      std::cerr << desc;
      return 1;
    }

  // setup log4cxx
  if (logging.empty())
    {
      init_loggers("dmcsGen");
    }
  else
    {
      init_loggers("dmcsGen", logging.c_str());
    }

  switch (topology_type)
    {
    case DIAMOND_DOWN_TOPOLOGY:
      { 
	if (no_contexts == 1 || no_contexts % 3 != 1)
	  {
	    std::cerr << "For \"Chain of diamond\" topology, the number of contexts must be 3n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case DIAMOND_ARBITRARY_TOPOLOGY:
      { 
	if (no_contexts == 1 || no_contexts % 3 != 1)
	  {
	    std::cerr << "For \"Chain of diamond\" topology, the number of contexts must be 3n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case DIAMOND_ZIGZAG_TOPOLOGY:
      { 
	if (no_contexts == 1 || no_contexts % 3 != 1)
	  {
	    std::cerr << "For \"Chain of zig-zag diamond\" topology, the number of contexts must be 3n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case PURE_RING_TOPOLOGY:
      { 
	if (no_contexts == 1)
	  {
	    std::cerr << "For \"Pure Ring\" topology, the number of contexts must be at least 2." << std::endl;
	    return 1;
	  }
      }
      break;
    case RING_EDGE_TOPOLOGY:
      { 
	if (no_contexts == 1)
	  {
	    std::cerr << "For \"Ring\" topology, the number of contexts must be at least 2." << std::endl;
	    return 1;
	  }
      }
      break;
    case BINARY_TREE_TOPOLOGY:
      { 
	if (no_contexts == 1)
	  {
	    std::cerr << "For \"binary tree\" topology, the number of contexts must be at least 2." << std::endl;
	    return 1;
	  }
      }
      break;
    case HOUSE_TOPOLOGY:
      { 
	if (no_contexts == 1 || no_contexts % 4 != 1)
	  {
	    std::cerr << "For \"House\" topology, the number of contexts must be 4n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case MULTIPLE_RING_TOPOLOGY:
      { 
	if ((no_contexts < 6) && (no_contexts % 3 != 0))
	  {
	    std::cerr << "For \"Multiple Ring\" topology, the number of contexts must be 3(n-1) where n >= 3." << std::endl;
	    return 1;
	  }
      }
      break;
    default:
      {
	std::cerr << "Topology type must be in the range of 0-8 inclusive" << std::endl;
	return 1;
      }
    } // switch

  DMCS_LOG_INFO("Number of contexts:                            " << no_contexts);
  DMCS_LOG_INFO("Number of atoms per context:                   " << no_atoms);
  DMCS_LOG_INFO("Number of maximum interface atoms per context: " << no_interface_atoms);
  DMCS_LOG_INFO("Number of maximum bridge rules per context:    " << no_bridge_rules);
  DMCS_LOG_INFO("Topology type:                                 " << topology_type);
  DMCS_LOG_INFO("Prefix for filename:                           " << prefix);

  return 0;
}



void
setup_topos()
{
  // initialize both original and optimal topologies. At the beginning
  // all neighbor vectors are empty.
  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      NeighborVecPtr tmp3(new NeighborVec);
      orig_topo->push_back(tmp3);

      NeighborVecPtr tmp4(new NeighborVec);
      opt_topo->push_back(tmp4);
    }
}



void 
init()
{
  genSignatures(sigmas, no_contexts, no_atoms);
  genInterface(context_interfaces, no_contexts, no_atoms, no_interface_atoms);
  setup_topos();

  // initialize minV as we now know the system size
  BeliefStatePtr someV(new BeliefState(no_contexts, BeliefSet()));
  minV = someV;

  // now set all epsilon bits to 1
  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      setEpsilon((*minV)[i]);
    }
}



void
generate_orig_topology()
{
  switch (topology_type)
    {
    case DIAMOND_DOWN_TOPOLOGY:
      {
	orig_topo_gen = new DiamondTopoGenerator(orig_topo);
	break;
      }
    case DIAMOND_ARBITRARY_TOPOLOGY:
      {
	orig_topo_gen = new DiamondArbitraryTopoGenerator(orig_topo);
	break;
      }
    case DIAMOND_ZIGZAG_TOPOLOGY:
      {
	orig_topo_gen = new DiamondZigZagTopoGenerator(orig_topo);
	break;
      }
    case PURE_RING_TOPOLOGY:
      {
	orig_topo_gen = new RingTopoGenerator(orig_topo);
	break;
      }
    case RING_EDGE_TOPOLOGY:
      {
	orig_topo_gen = new RingEdgeTopoGenerator(orig_topo);
	break;
      }
    case BINARY_TREE_TOPOLOGY:
      {
	orig_topo_gen = new BinaryTreeTopoGenerator(orig_topo);
	break;
      }
    case HOUSE_TOPOLOGY:
      {
	orig_topo_gen = new HouseTopoGenerator(orig_topo);
	break;
      }
    case MULTIPLE_RING_TOPOLOGY:
      {
	orig_topo_gen = new MultipleRingTopoGenerator(orig_topo);
	break;
      }
    }

  orig_topo_gen->generate();


#if defined(DEBUG)
  DMCS_LOG_DEBUG("Original topology:");
  for (std::size_t i = 1; i <= no_contexts; ++ i)
    {
      NeighborVecPtr neighbors = (*orig_topo)[i-1];

      std::ostringstream oss;
      std::copy(neighbors->begin(), neighbors->end(), std::ostream_iterator<std::size_t>(oss, " "));

      DMCS_LOG_DEBUG(i << " --> " << oss.str());
    }
#endif
}



// including generating interface and writing to files
void
generate_contexts()
{
  ContextGenerator cgen(orig_topo, context_interfaces, 
			sigmas, minV, lcim, no_atoms, no_bridge_rules, 
			topology_type, prefix);

  cgen.generate();
  // After this, we have local_kb, bridge rules of all contexts and
  // minV of the system set up. Furthermore, we have the map from
  // edsges to local interfaces, which will be used to compte the
  // interface in the optimal topology.

#ifdef DEBUG
  DMCS_LOG_DEBUG("minV: " << minV);
  DMCS_LOG_DEBUG("Original local interface:");

  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
    {
      ContextPair cp = it->first;

      DMCS_LOG_DEBUG("(" << cp.first << ", " << cp.second << ") --> " << it->second);
    }
#endif
}



void
generate_opt_topology()
{
  // copy the content of lcim to opt_lcim
  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
    {
      ContextPair cp = it->first;
      
      BeliefStatePtr interface(new BeliefState(*it->second));
      LocalInterfacePair lp(cp, interface);
      opt_lcim->insert(lp);
    }

  // now the respective optimal topology generator will take care of
  // tweaking opt_topo and opt_lcim to build the corresponding optimal
  // query plan.

  switch (topology_type)
    {
    case DIAMOND_DOWN_TOPOLOGY:
      {
	opt_topo_gen = new DiamondOptTopoGenerator(no_contexts, opt_lcim);
	break;
      }
    case DIAMOND_ZIGZAG_TOPOLOGY:
      {
	opt_topo_gen = new DiamondZigZagOptTopoGenerator(no_contexts, opt_lcim);
	break;
      }
    case PURE_RING_TOPOLOGY:
      {
	opt_topo_gen = new RingOptTopoGenerator(no_contexts, opt_lcim);
	break;
      }
    case HOUSE_TOPOLOGY:
      {
	opt_topo_gen = new HouseOptTopoGenerator(no_contexts, opt_lcim);
	break;
      }
    case MULTIPLE_RING_TOPOLOGY:
      {
	opt_topo_gen = new MultipleRingOptTopoGenerator(no_contexts, opt_lcim);
	break;
      }
    }

  // then adjust to get optimal local interface
  opt_topo_gen->create_opt_interface();

#ifdef DEBUG
  DMCS_LOG_DEBUG("Optimal local interface:");

  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
    {
      ContextPair cp = it->first;

      DMCS_LOG_DEBUG("(" << cp.first << ", " << cp.second << ") --> " << it->second);
    }
#endif
}


void
generate_query_plan(QueryPlanPtr query_plan, LocalInterfaceMapPtr lcim)
{
  DMCS_LOG_TRACE("add_vertex");

  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      boost::add_vertex(query_plan->graph);
    }

  DMCS_LOG_TRACE("setup properties");

  std::string localhost = "localhost";
  std::stringstream out;
  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      out.str("");
      out << BASE_PORT + i;
      std::string port = out.str();

      putProp<VertexHostnameProperty, std::string>(i, query_plan->hostname, localhost);
      putProp<VertexPortProperty, std::string>(i, query_plan->port, port);
      putProp<VertexSigmaProperty, Signature>(i, query_plan->sigma, *(*sigmas)[i-1]);
    }

  DMCS_LOG_TRACE("setup properties");

  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
    {
      ContextPair cp = it->first;
      std::size_t from = cp.first;
      std::size_t to   = cp.second;
      BeliefStatePtr interface = it->second;

      //add_edge(from, to, orig_qp);
      Vertex u = boost::vertex(from-1, query_plan->graph);
      Vertex v = boost::vertex(to-1, query_plan->graph);
      
      boost::add_edge(u, v, query_plan->graph);
      putProp<EdgeInterfaceProperty, BeliefStatePtr>(query_plan, from, to, query_plan->interface, interface);
    }

  DMCS_LOG_TRACE("write graphviz");

  out.str("");
  out << minV;
  boost::write_graphviz(std::cerr, query_plan->graph, 
			make_vertex_writer(query_plan->hostname, query_plan->port, query_plan->sigma), 
			make_edge_writer(query_plan->interface),
			make_graph_writer(out.str()));
}



void
print_query_plan(QueryPlanPtr query_plan, const std::string& filename)
{
  std::ofstream file_topo;
  file_topo.open(filename.c_str());
  std::stringstream out;
  out << minV;
  boost::write_graphviz(file_topo, query_plan->graph, 
			make_vertex_writer(query_plan->hostname, query_plan->port, query_plan->sigma), 
			make_edge_writer(query_plan->interface),
			make_graph_writer(out.str()));
  file_topo.close();
}



void
print_command_lines()
{
  std::string filename_command_line        = prefix + CMD_EXT;
  std::string filename_command_line_sh     = prefix + SH_CMD_EXT;

  file_command_line.open(filename_command_line.c_str());
  file_command_line_sh.open(filename_command_line_sh.c_str());

  // Initialization for shell scripts
  file_command_line_sh << "#!/bin/bash" << std::endl
		       << "export TIMEFORMAT=$'\\nreal\\t%3R\\nuser\\t%3U\\nsys\\t%3S'" << std::endl
		       << "export TESTSPATH='" TESTSDIR "'" << std::endl
		       << "export DMCSPATH='.'" << std::endl;

  // dmcsd commands
  // ./dmcsd <id> <hostname> <port> <filename_lp> <filename_br> <filename_topo> 

  std::string command_line_sh;
  std::string command_line;

  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      std::stringstream out;
      std::stringstream index;
      std::stringstream port;
      
      index << i;
      port << BASE_PORT + i;

      // ID PortNo@Hostname
      out << "--" << CONTEXT_ID << "=" << i << " " 
	  << "--" << PORT << "=" << port.str();

      command_line_sh = "$DMCSPATH/" DMCSD " " + out.str() + 
	" --" KB "=$TESTSPATH/" + prefix + "-" + index.str() + LP_EXT +
	" --" BR "=$TESTSPATH/" + prefix + "-" + index.str() + BR_EXT +
	" --" TOPOLOGY "=$TESTSPATH/" + prefix + TOP_EXT;

      command_line = "./" DMCSD " " + out.str() + 
	" --" KB "=" TESTSDIR "/" + prefix + "-" + index.str() + LP_EXT +
	" --" BR "=" TESTSDIR "/" + prefix + "-" + index.str() + BR_EXT +
	" --" TOPOLOGY "=" TESTSDIR "/" + prefix + TOP_EXT;

      file_command_line_sh << command_line_sh << " >/dev/null 2>&1 &" << std::endl;
      file_command_line << command_line << std::endl;
    }

  // client command
  // time | ./dmcsc localhost 5001
  std::stringstream port1;
  std::stringstream system_size;
  std::stringstream globalV;

  port1 << 5001;
  system_size << no_contexts;
  globalV << minV;

  command_line_sh = "/usr/bin/time --portability -o "+ prefix +"-dmcs-time.log $DMCSPATH/"  DMCSC
    " --" HOSTNAME "=localhost"
    " --" PORT "=" + port1.str() +
    " --" SYSTEM_SIZE "=" + system_size.str() + 
    " --" QUERY_VARS "=\"" + globalV.str() + "\" > "+ prefix +"-dmcs.log 2> " + prefix +"-dmcs-err.log";
  
  command_line = "time ./" DMCSC 
    " --" HOSTNAME "=localhost" 
    " --" PORT "=" + port1.str() +
    " --" SYSTEM_SIZE "=" + system_size.str() + 
    " --" QUERY_VARS "=\"" + globalV.str() + "\"";

  file_command_line_sh << command_line_sh << std::endl << "killall " DMCSD << std::endl;
  file_command_line << command_line << std::endl;

  file_command_line_sh.close();
  file_command_line.close();
}


void
print_opt_command_lines()
{
  std::string filename_command_line_opt    = prefix + OPT_CMD_EXT;
  std::string filename_command_line_opt_sh = prefix + OPT_SH_CMD_EXT;

  file_command_line_opt.open(filename_command_line_opt.c_str());
  file_command_line_opt_sh.open(filename_command_line_opt_sh.c_str());

  // Initialization for shell scripts
  file_command_line_opt_sh << "#!/bin/bash" << std::endl
			   << "export TIMEFORMAT=$'\\nreal\\t%3R\\nuser\\t%3U\\nsys\\t%3S'" << std::endl
			   << "export TESTSPATH='" TESTSDIR "'" << std::endl
			   << "export DMCSPATH='.'" << std::endl;

  // dmcsd commands
  // ./dmcsd <id> <hostname> <port> <filename_lp> <filename_br> <filename_topo> 

  std::string command_line_opt_sh;
  std::string command_line_opt;

  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      std::stringstream out;
      std::stringstream index;
      std::stringstream port;
      
      index << i;
      port << BASE_PORT + i;

      // ID PortNo@Hostname
      out << "--" << CONTEXT_ID << "=" << i << " " 
	  << "--" << PORT << "=" << port.str();

      command_line_opt_sh = "$DMCSPATH/" DMCSD " " + out.str() + 
	" --" KB "=$TESTSPATH/" + prefix + "-" + index.str() + LP_EXT +
	" --" BR "=$TESTSPATH/" + prefix + "-" + index.str() + BR_EXT +
	" --"TOPOLOGY "=$TESTSPATH/" + prefix + OPT_EXT;

      command_line_opt = "./" DMCSD " " + out.str() + 
	" --" KB "=" TESTSDIR "/" + prefix + "-" + index.str() + LP_EXT +
	" --" BR "=" TESTSDIR "/" + prefix + "-" + index.str() + BR_EXT +
	" --" TOPOLOGY "=" TESTSDIR "/" + prefix + OPT_EXT;

      file_command_line_opt_sh << command_line_opt_sh << " >/dev/null 2>&1 &" << std::endl;
      file_command_line_opt << command_line_opt << std::endl;
    }

  // client command
  // time | ./dmcsc localhost 5001
  std::stringstream port1;
  std::stringstream system_size;
  std::stringstream globalV;

  port1 << 5001;
  system_size << no_contexts;
  globalV << minV;

  command_line_opt_sh = "/usr/bin/time --portability -o "+ prefix +"-dmcsopt-time.log $DMCSPATH/"  DMCSC
    " --" HOSTNAME "=localhost" 
    " --" PORT "=" + port1.str() +
    " --" SYSTEM_SIZE "=" + system_size.str() + " > "+ prefix +"-dmcsopt.log 2> " + prefix +"-dmcsopt-err.log";
  
  command_line_opt = "time ./"  DMCSC 
    " --" HOSTNAME "=localhost"
    " --" PORT "=" + port1.str() +
    " --" SYSTEM_SIZE "=" + system_size.str();
  
  file_command_line_opt_sh << command_line_opt_sh << std::endl << "killall " DMCSD << std::endl;
  file_command_line_opt << command_line_opt << std::endl;

  file_command_line_opt_sh.close();
  file_command_line_opt.close();
}


const std::string
getOptimumDLVFilter() 
{
  std::string result;

  // just want to see the result from the root context and its neighbors
  NeighborVecPtr neighbors = (*opt_topo)[0];
  BeliefStatePtr neighborsInterface(new BeliefState(no_contexts, 0));
  
  for (NeighborVec::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
    {
      const BeliefStatePtr& currentInterface = getInterface(opt_lcim, 1, *it);

      BeliefState::iterator jt = currentInterface->begin();
      BeliefState::iterator kt = neighborsInterface->begin();

      for (; jt != currentInterface->end(); ++jt, ++kt)
	{
	  *kt |= *jt;
	}
    }

  BeliefState::iterator kt = neighborsInterface->begin();
  *kt |= std::numeric_limits<unsigned long>::max();
  
  if (topology_type == DIAMOND_DOWN_TOPOLOGY)
    {
      const BeliefStatePtr& interface24 =  getInterface(opt_lcim, 2, 4);
      BeliefState::iterator jt = interface24->begin();
      BeliefState::iterator kt = neighborsInterface->begin();
      
      for (; jt != interface24->end(); ++jt,++kt)
	{
	  *kt |= *jt;
	}
      
      const BeliefStatePtr& interface34 =  getInterface(opt_lcim, 3, 4);
      BeliefState::iterator jt1 = interface34->begin();
      BeliefState::iterator kt1 = neighborsInterface->begin();
      
      for (; jt1 != interface34->end(); ++jt1,++kt1)
	{
	  *kt1 |= *jt1;
	}
      
    }
  else if (topology_type == DIAMOND_ZIGZAG_TOPOLOGY)
    {
      const BeliefStatePtr& interface23 =  getInterface(opt_lcim, 2, 3);
      BeliefState::iterator jt1 = interface23->begin();
      BeliefState::iterator kt1 = neighborsInterface->begin();
      
      for (; jt1 != interface23->end(); ++jt1,++kt1)
	{
	  *kt1 |= *jt1;
	}
    }
  
  
  bool first = true;
  
  for (std::size_t contextID = 1; contextID <= no_contexts; ++contextID)
    {
      const SignaturePtr contextSignature  = (*sigmas)[contextID-1];
      const SignatureByLocal& contextSigByLocal = boost::get<Tag::Local>(*contextSignature);

      BeliefSet contextInterface = (*neighborsInterface)[contextID-1];
      SignatureByLocal::const_iterator context_it; 
      
      if (!isEpsilon(contextInterface))
	{
	  for (std::size_t i = 1; // ignore epsilon bit
	       i < sizeof(contextInterface)*8 ; ++i)
	    {
	      if (testBeliefSet(contextInterface, i) && i <= contextSignature->size())
		{
		  context_it = contextSigByLocal.find(i); 
		  assert(context_it != contextSigByLocal.end());
		  if (!first)
		    {
		      result += ",";
		    }
		  else 
		    {
		      first = false;
		    }
		  result += context_it->sym;
		}
	    }
	}
    }
  
  return result;
}


std::string
getGeneralDLVFilter()
{
  std::string result; 
  bool first = true;
  
  for (std::size_t contextID = 1; contextID <= no_contexts; ++contextID)
    {
      const SignaturePtr contextSignature  = (*sigmas)[contextID-1];
      const SignatureByLocal& contextSigByLocal = boost::get<Tag::Local>(*contextSignature);

      BeliefSet contextInterface = (*minV)[contextID-1];
      SignatureByLocal::const_iterator context_it; 

      if (!isEpsilon(contextInterface))
	{
	  for (std::size_t i = 1; // ignore epsilon bit
	       i < sizeof(contextInterface)*8;
	       ++i)
	    {
	      if (contextID != 1)
		{
		  if (testBeliefSet(contextInterface, i))
		    {
		      context_it = contextSigByLocal.find(i);
		      assert(context_it != contextSigByLocal.end());
		      if (!first)
			{
			  result += ",";
			}
		      else 
			{
			  first = false;
			}
		      result += context_it->sym;
		    }
		} // this extra 'if' is used to make sure that all of sigma 1 is used, regardless of global V
	      else if (i <= contextSignature->size())
		{
		  context_it = contextSigByLocal.find(i); 
		  assert(context_it != contextSigByLocal.end());
		  if (!first)
		    {
		      result += ",";
		    }
		  else 
		    {
		      first = false;
		    }
		  result += context_it->sym;
		}
	    }
	}
    }
  return result;  
}

void
print_dlv_command_lines()
{
  std::string filename_command_line_dlv      = prefix + DLV_CMD_EXT;
  file_command_line_dlv.open(filename_command_line_dlv.c_str());

  std::string generalFilter = getGeneralDLVFilter();
  file_command_line_dlv << "dlv -silent -filter=" << generalFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq "<< std::endl;
  file_command_line_dlv << "dlv -silent -filter=" << generalFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq | wc -l"<< std::endl;

  file_command_line_dlv.close();
}


void
print_opt_dlv_command_lines()
{
  std::string filename_command_line_dlv_opt  = prefix + OPT_DLV_CMD_EXT;
  file_command_line_dlv_opt.open(filename_command_line_dlv_opt.c_str());

  std::string optimumFilter = getOptimumDLVFilter();
  file_command_line_dlv_opt << "dlv -silent -filter=" << optimumFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq "<< std::endl;
  file_command_line_dlv_opt << "dlv -silent -filter=" << optimumFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq | wc -l"<< std::endl;

  file_command_line_dlv_opt.close();
}


int 
main(int argc, char* argv[])
{
  if (read_input(argc, argv) == 1) // reading input not completed
    {
      return 1;
    }

  // this is a very bad idea, it will only change the seed every second!
  // srand( time(NULL) );
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + tv.tv_usec);
  }

  init();

  DMCS_LOG_TRACE("generate_orig_topology");
  generate_orig_topology();

  DMCS_LOG_TRACE("generate_contexts");
  generate_contexts();

  DMCS_LOG_TRACE("generate_query_plan");
  generate_query_plan(orig_qp, lcim);

  DMCS_LOG_TRACE("print_query_plan");
  print_query_plan(orig_qp, prefix + TOP_EXT);

  DMCS_LOG_TRACE("print_command_lines");
  print_command_lines();

  DMCS_LOG_TRACE("print_dlv_command_lines");
  print_dlv_command_lines();

  // binary tree topology is a special case, orig_qp can be reused to
  // print opt_qp. Hence, we just need to print without any further
  // construction for the opt case.
  if (topology_type == BINARY_TREE_TOPOLOGY)
    {
      print_query_plan(orig_qp, prefix + OPT_EXT);
      print_opt_command_lines();
      print_opt_dlv_command_lines();
    }

  // only for some fixed topologies where optimization is possible
  if (topology_type != RANDOM_TOPOLOGY && topology_type != DIAMOND_ARBITRARY_TOPOLOGY &&
      topology_type != RING_EDGE_TOPOLOGY && topology_type != BINARY_TREE_TOPOLOGY)
    {
      generate_opt_topology();
      generate_query_plan(opt_qp, opt_lcim);
      print_query_plan(opt_qp,  prefix + OPT_EXT);
      print_opt_command_lines();
      print_opt_dlv_command_lines();
    }

  return 0;
}
