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
 * @date   Fri Oct  26 22:00:00 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "generator/new_dmcsGen.h"
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
#include "generator/NewContextGenerator.h"
#include "generator/QueryPlanWriter.h"

#include "dmcs/ProgramOptions.h"
#include "dmcs/QueryPlan.h"
#include "dmcs/Log.h"

#include "mcs/BeliefTable.h"
#include "mcs/BeliefStateOffset.h"
#include "mcs/NewBeliefState.h"

#include "mcs/Rule.h"
#include "mcs/Signature.h"

#include <boost/program_options.hpp>

#include <sys/time.h>

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

using namespace dmcs;
using namespace dmcs::generator;

#define DMCSD "new_dmcsd"
#define DMCSC "new_dmcsc"
#define DMCSM "new_dmcsm"
#define TESTDIR "."
//#define DMCSPATH "../build-dbg/src"
#define STR_LOCALHOST "localhost"
#define LP_EXT  ".lp"
#define BR_EXT  ".br"
#define QP_EXT  ".qp"
#define TOP_EXT ".top"
#define OPT_EXT ".opt"
#define ANS_EXT ".ans"
#define CMD_EXT "_command_line.txt"
#define SH_CMD_EXT "_command_line.sh"
#define OPT_CMD_EXT "_command_line_opt.txt"
#define OPT_SH_CMD_EXT "_command_line_opt.sh"
#define STREAMING_SH_CMD_EXT "_command_line_streaming.sh"
#define DLV_EXT ".dlv"
#define DLV_CMD_EXT "_dlv.txt"
#define OPT_DLV_CMD_EXT "_dlv_opt.txt"

#define MANAGER_PORT 4999
#define BASE_PORT    5000

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

typedef std::map<std::string, std::string> OptionValueMap;
OptionValueMap cmdline_options;

BeliefTableVecPtr sigma_vec(new BeliefTableVec);
BeliefTableVecPtr context_interface_table_vec(new BeliefTableVec);

NewBeliefStatePtr new_minV;

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

bool        gen_data;
std::size_t no_contexts;
std::size_t no_atoms;
std::size_t no_interface_atoms;
std::size_t no_bridge_rules;
std::size_t topology_type;
std::size_t startup_time;
std::size_t pack_size;
std::size_t timeout;

std::string prefix;
std::string filename;
std::string logging;
std::string dmcspath;

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
    (GENDATA, boost::program_options::value<bool>(&gen_data)->default_value(false), "Set to generate data. Generate only command lines by default.")
    (CONTEXTS, boost::program_options::value<std::size_t>(&no_contexts)->default_value(4), "Number of contexts")
    (ATOMS, boost::program_options::value<std::size_t>(&no_atoms)->default_value(9), "Number of local atoms")
    (INTERFACE, boost::program_options::value<std::size_t>(&no_interface_atoms)->default_value(3), "Number of interface atoms")
    (BRIDGE_RULES, boost::program_options::value<std::size_t>(&no_bridge_rules)->default_value(2), "Number of bridge rules")
    (TOPOLOGY, boost::program_options::value<std::size_t>(&topology_type)->default_value(1), HELP_MESSAGE_TOPO)
    (PREFIX, boost::program_options::value<std::string>(&prefix)->default_value("student"), "Prefix for all files")
    (DMCSPATH, boost::program_options::value<std::string>(&dmcspath), "Path to dmcs binaries")
    (STARTUP_TIME, boost::program_options::value<std::size_t>(&startup_time)->default_value(20), "Sleeping time after initializing all dmcsd")
    (PACK_SIZE, boost::program_options::value<std::size_t>(&pack_size)->default_value(0), "Package size")
    (TIMEOUT, boost::program_options::value<std::size_t>(&timeout)->default_value(600), "Set timeout when running the test")
    (LOGGING, boost::program_options::value<std::string>(&logging)->default_value(""), "log4cxx config file")
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count(HELP) || 
      prefix.compare("") == 0 || dmcspath.compare("") == 0 || 
      no_contexts == 0 || no_atoms == 0 ||
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
add_cmdline_options()
{
  std::stringstream out;
  out << MANAGER_PORT;
  std::string str_manager = "localhost:" + out.str();

  out.str("");
  out << no_contexts;
  std::string str_system_size = out.str();

  out.str("");
  out << no_atoms;
  std::string str_bs_size = out.str();

  cmdline_options.insert(std::pair<std::string, std::string>("manager", str_manager));
  cmdline_options.insert(std::pair<std::string, std::string>("context", ""));
  cmdline_options.insert(std::pair<std::string, std::string>("port", ""));
  cmdline_options.insert(std::pair<std::string, std::string>("system-size", str_system_size));
  cmdline_options.insert(std::pair<std::string, std::string>("belief-state-size", str_bs_size));
  cmdline_options.insert(std::pair<std::string, std::string>("kb", prefix + ".lp"));
  cmdline_options.insert(std::pair<std::string, std::string>("br", prefix + ".br"));
  cmdline_options.insert(std::pair<std::string, std::string>("queryplan", prefix + ".qp"));
  cmdline_options.insert(std::pair<std::string, std::string>("returnplan", prefix + ".rp"));
}



void 
init()
{
  BeliefStateOffset* bso = BeliefStateOffset::create(no_contexts, no_atoms);

  genBeliefTables(sigma_vec, no_contexts, no_atoms);
  genInterface(context_interfaces, context_interface_table_vec, 
	       no_contexts, no_atoms, no_interface_atoms, sigma_vec);

  setup_topos();
  
  // initialize minV as we now know the system size
  new_minV = NewBeliefStatePtr(new NewBeliefState(no_contexts, no_atoms));
  const std::vector<std::size_t>& starting_offset = bso->getStartingOffsets();
  new_minV->setEpsilon(starting_offset);

  std::size_t last_index = no_atoms;
  if (no_contexts > 1)
    {
      last_index = starting_offset[1] - 1;
    }

  // turn on all bits of the root context
  for (std::size_t i = 1; i <= last_index; ++i)
    {
      new_minV->set(i);
    }

  add_cmdline_options();
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
  for (std::size_t i = 0; i < no_contexts; ++ i)
    {
      NeighborVecPtr neighbors = (*orig_topo)[i];

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
  NewContextGenerator cgen(orig_topo, context_interfaces, 
			   sigma_vec, new_minV, lcim, no_atoms, no_bridge_rules, 
			   topology_type, prefix);

  cgen.generate();
  // After this, we have local_kb, bridge rules of all contexts and
  // minV of the system set up. Furthermore, we have the map from
  // edsges to local interfaces, which will be used to compute the
  // interface in the optimal topology.

#ifdef DEBUG
  DMCS_LOG_TRACE("minV: " << *new_minV);
  DMCS_LOG_TRACE("Original local interface:");

  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
    {
      ContextPair cp = it->first;

      DMCS_LOG_TRACE("(" << cp.first << ", " << cp.second << ") --> " << *(it->second));
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
      
      NewBeliefStatePtr interface(new NewBeliefState(*it->second));
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

  for (LocalInterfaceMap::const_iterator it = opt_lcim->begin(); it != opt_lcim->end(); ++it)
    {
      ContextPair cp = it->first;

      DMCS_LOG_DEBUG("(" << cp.first << ", " << cp.second << ") --> " << *(it->second));
    }
#endif
}



void
write_belief_table_to_file(BeliefTablePtr btab, std::ofstream& file, 
			   const std::string& signature_kind,
			   std::size_t ctx_id)
{
  std::size_t port = BASE_PORT + ctx_id;

  file << "  {\n";
  file << "    ContextId: " << ctx_id << ",\n";
  file << "    HostName: \"localhost\",\n";
  file << "    Port: " << port << ",\n";
  file << "    " << signature_kind << ":\n";
  file << btab->gen_print();
  file << "  },\n";
}


BeliefTablePtr
input_signature_from_belief_state(std::size_t ctx_id,
				  NewBeliefStatePtr bs)
{
  BeliefTablePtr insig(new BeliefTable);
  BeliefTablePtr local_sig = (*sigma_vec)[ctx_id];

  const std::vector<std::size_t>& starting_offset = BeliefStateOffset::instance()->getStartingOffsets();
  std::size_t check_bit = starting_offset[ctx_id];
  std::size_t last_bit;
  if (ctx_id == no_contexts - 1)
    {
      last_bit = bs->size();
    }
  else
    {
      last_bit = starting_offset[ctx_id+1] - 1;
    }

  do
    {
      check_bit = bs->getNext(check_bit);
      if (check_bit > last_bit || check_bit == 0) 
	break;

      std::size_t local_id = check_bit - starting_offset[ctx_id];
      ID id(ctx_id | ID::MAINKIND_BELIEF, local_id);
      const Belief& b = local_sig->getByID(id);
      insig->storeWithID(b, id);
    }
  while (1);

  return insig;
}


void
write_return_signature_to_file(std::ofstream& file, 
			       NewBeliefStatePtr bs,
			       std::size_t ctx_id)
{
  const std::vector<std::size_t>& starting_offset = BeliefStateOffset::instance()->getStartingOffsets();
  std::size_t count = 0;
  std::size_t epsilon_id;
  std::size_t last_bit;
  std::size_t check_bit;
  std::size_t local_id;

  file << "  {\n";
  file << "    ContextId: " << ctx_id << ",\n";
  file << "    ReturnSignature:\n";
  file << "    [\n";
  for (std::vector<std::size_t>::const_iterator it = starting_offset.begin(); it != starting_offset.end(); ++it)
    {
      epsilon_id = *it;
      if (bs->test(epsilon_id) == NewBeliefState::DMCS_TRUE)
	{
	  file << "      {\n";
	  file << "        ContextId: " << count << ",\n";
	  file << "        ReturnBeliefs: [";
	  if (count == no_contexts - 1)
	    {
	      last_bit = bs->size();
	    }
	  else
	    {
	      last_bit = starting_offset[count+1]-1;
	    }
	  
	  check_bit = bs->getNext(epsilon_id);
	  if (check_bit != 0 && check_bit <= last_bit)
	    {
	      local_id = check_bit - epsilon_id;
	      file << local_id;
	      do
		{
		  check_bit = bs->getNext(check_bit);
		  if (check_bit > last_bit || check_bit == 0) 
		    break;
		  local_id = check_bit - epsilon_id;
		  file << ", " << local_id;
		}
	      while (1);
	    }

	  file << "]\n";
	  file << "      },\n";
	}
      count++;
    }
  file << "   ]\n";
  file << "  },\n";
}


void
write_plans(bool write_opt_plans)
{
  std::string local_signature = "LocalSignature";
  std::string input_signature = "InputSignature";

  // client's query plan ######################################################################################
  std::string filename_client_qp = "client.qp";
  std::ofstream file_client_qp;
  file_client_qp.open(filename_client_qp.c_str());
  file_client_qp << "[\n";
  
  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      write_belief_table_to_file((*sigma_vec)[i], file_client_qp, local_signature, i);
    }

  file_client_qp << "]\n";
  file_client_qp.close();

  // context specific query plans #############################################################################

  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      std::stringstream str_i;
      str_i << i;

      // query plans ##########################################################################################
      std::string filename_qp = prefix + "-" + str_i.str() + ".qp";
      std::ofstream file_qp;
      file_qp.open(filename_qp.c_str());
      file_qp << "[\n";
      write_belief_table_to_file((*sigma_vec)[i], file_qp, local_signature, i);

      for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
	{
	  ContextPair cp = it->first;
	  if (cp.first == i)
	    {
	      BeliefTablePtr insig = input_signature_from_belief_state(cp.second, it->second);
	      write_belief_table_to_file(insig, file_qp, input_signature, cp.second);
	    }
	}

      file_qp << "]\n";
      file_qp.close();

      // return plans #########################################################################################
      std::string filename_rp = prefix + "-" + str_i.str() + ".rp";
      std::ofstream file_rp;
      file_rp.open(filename_rp.c_str());
      file_rp << "[\n";
      if (i == 0)
	{
	  write_return_signature_to_file(file_rp, new_minV, 1023);
	}
      else
	{
	  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
	    {
	      ContextPair cp = it->first;
	      if (cp.second == i)
		{
		  write_return_signature_to_file(file_rp, new_minV, cp.first);
		}
	    }
	}
      file_rp << "]\n";
      file_rp.close();      

      // opt return plans #####################################################################################
      if (write_opt_plans)
	{
	  std::string filename_opt_rp = prefix + "-" + str_i.str() + ".orp";
	  std::ofstream file_opt_rp;
	  file_opt_rp.open(filename_opt_rp.c_str());
	  file_opt_rp << "[\n";
	  if (i == 0)
	    {
	      const std::vector<std::size_t>& starting_offset = BeliefStateOffset::instance()->getStartingOffsets();
	      NewBeliefStatePtr bs(new NewBeliefState(no_contexts, no_atoms));
	      for (std::size_t j = 0; j < starting_offset[1]; ++j)
		bs->set(j);
	      
	      write_return_signature_to_file(file_opt_rp, bs, 1023);
	    }
	  else
	    {
	      for (LocalInterfaceMap::const_iterator it = opt_lcim->begin(); it != opt_lcim->end(); ++it)
		{
		  ContextPair cp = it->first;
		  if (cp.second == i)
		    {
		      NewBeliefStatePtr bs = it->second;
		      write_return_signature_to_file(file_opt_rp, bs, cp.first);
		    }
		}
	    }
	  file_opt_rp << "]\n";
	  file_opt_rp.close();
	}

    } // end for i = 0..no_contexts-1
}


void
print_dmcsd_line(bool is_shellscript,
		 std::size_t i,
		 const std::string& testpath,
		 const std::string& path,
		 std::ofstream& file,
		 bool want_log = false)
{
  std::stringstream out;
  std::string final_value;

  file << path << "/" << DMCSD << " ";

  for (OptionValueMap::const_iterator it = cmdline_options.begin(); it != cmdline_options.end(); ++it)
    {
      out.str("");
      const std::string& option = it->first;
      std::string value = it->second;

      if (value == "")
	// need some number
	{
	  if (option == "context")
	    {
	      out << i;
	      final_value = out.str();
	    }
	  else // option == "port"
	    {
	      std::size_t port = BASE_PORT + i;
	      out << port;
	      final_value = out.str();
	    }
	}
      else
	{
	  // need to adapt a file name with the context id
	  std::size_t dot_pos = value.find(".");
	  if (dot_pos != std::string::npos)
	    {
	      out << i;
	      value.insert(dot_pos, "-" + out.str());
	      final_value = testpath + "/" + value;
	    }
	  else
	    {
	      final_value = value;
	    }
	}
      file << "--" << option << "=" << final_value << " ";
    } // end for
  
  if (is_shellscript)
    {
      if (want_log)
	{
	  // find test name from option --kb
	  OptionValueMap::const_iterator it = cmdline_options.find("kb");
	  assert (it != cmdline_options.end());

	  std::stringstream out;
	  out << i;
	  
	  std::string filename = it->second;
	  std::size_t dot_pos = filename.find(".");
	  assert (dot_pos != std::string::npos);

	  std::string logfilename = filename.substr(0, dot_pos) + "-" + out.str() + ".log";

	  file << " >" << logfilename << " 2>&1 &";
	}
      else
	file << " >/dev/null 2>&1 &";
    }

  file << std::endl;
}


void
print_command_lines_file(bool is_shellscript, 
			 std::string testpath,
			 std::string path,
			 std::size_t k1,
			 std::size_t k2,
			 std::string filename,
			 bool want_log = false)
{
  std::ofstream file;
  file.open(filename.c_str());

  if (is_shellscript)
    {
      file << "#!/bin/bash" << std::endl
	   << "export TIMEFORMAT=$'\\nreal\\t%3R\\nuser\\t%3U\\nsys\\t%3S'" << std::endl
	   << "export TIMEOUT=" << timeout << std::endl
	   << "export TESTPATH='" << testpath << "'" << std::endl
	   << "export DMCSPATH='" << path << "'" << std::endl
	   << "killall " << DMCSD << std::endl << std::endl
	   << "killall " << DMCSM << std::endl << std::endl;

      testpath = "$TESTPATH";
      path = "$DMCSPATH";
    }

  file << path << "/" << DMCSM " " 
       << "--port=" << MANAGER_PORT << " "
       << "--system-size=" << no_contexts << " &" << std::endl;

  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      print_dmcsd_line(is_shellscript, i, testpath, path, file, want_log);
    }

  if (is_shellscript)
    {
      file << "sleep " << startup_time << std::endl;
    }

  file << "/usr/bin/time --portability -o " << prefix << "-time.log ";

  if (is_shellscript)
    {
      file << "/usr/bin/timeout -k 20 $TIMEOUT ";
    }

  file << path << "/" << DMCSC << " "
       << "--hostname=localhost "
       << "--port=" << BASE_PORT << " "
       << "--root=0 "
       << "--signature=" << testpath << "/client.qp "
       << "--belief-state-size=" << no_atoms << " "
       << "--k1=" << k1 << " "
       << "--k2=" << k2 << " "
       << " > " << prefix << ".log "
       << " 2> " << prefix << "-err.log" << std::endl;

  file.close();
}


void
print_command_lines()
{
  std::string filename_command_line_all            = prefix + "_command_line_all.txt";
  std::string filename_command_line_all_sh         = prefix + "_command_line_all.sh";
  std::string filename_command_line_all_log_sh     = prefix + "_command_line_all_log.sh";
  std::string filename_command_line_opt_all        = prefix + "_command_line_opt_all.txt";
  std::string filename_command_line_opt_all_sh     = prefix + "_command_line_opt_all.sh";
  std::string filename_command_line_opt_all_log_sh = prefix + "_command_line_opt_all_log.sh";

  print_command_lines_file(false, TESTDIR, dmcspath, 0, 0, filename_command_line_all);
  print_command_lines_file(true, TESTDIR, dmcspath, 0, 0, filename_command_line_all_sh);
  print_command_lines_file(true, TESTDIR, dmcspath, 0, 0, filename_command_line_all_log_sh, true);

  if (pack_size > 0)
    {
      std::stringstream out;
      out << pack_size;

      std::string filename_command_line_pack        = prefix + "_command_line_" + out.str() + ".txt";
      std::string filename_command_line_pack_sh     = prefix + "_command_line_" + out.str() + ".sh";
      std::string filename_command_line_pack_log_sh = prefix + "_command_line_log_" + out.str() + ".sh";
      print_command_lines_file(false, TESTDIR, dmcspath, 1, pack_size, filename_command_line_pack);
      print_command_lines_file(true, TESTDIR, dmcspath, 1, pack_size, filename_command_line_pack_sh);
      print_command_lines_file(true, TESTDIR, dmcspath, 1, pack_size, filename_command_line_pack_log_sh, true);
    }

  OptionValueMap::iterator it = cmdline_options.find("returnplan");
  assert (it != cmdline_options.end());
  it->second = prefix + ".orp";

  print_command_lines_file(false, TESTDIR, dmcspath, 0, 0, filename_command_line_opt_all);
  print_command_lines_file(true, TESTDIR, dmcspath, 0, 0, filename_command_line_opt_all_sh);
  print_command_lines_file(true, TESTDIR, dmcspath, 0, 0, filename_command_line_opt_all_log_sh, true);

  if (pack_size > 0)
    {
      std::stringstream out;
      out << pack_size;
      std::string filename_command_line_opt_pack        = prefix + "_command_line_opt_" + out.str() + ".txt";
      std::string filename_command_line_opt_pack_sh     = prefix + "_command_line_opt_" + out.str() + ".sh";
      std::string filename_command_line_opt_pack_log_sh = prefix + "_command_line_opt_log_" + out.str() + ".sh";

      print_command_lines_file(false, TESTDIR, dmcspath, 1, pack_size, filename_command_line_opt_pack);
      print_command_lines_file(true, TESTDIR, dmcspath, 1, pack_size, filename_command_line_opt_pack_sh);
      print_command_lines_file(true, TESTDIR, dmcspath, 1, pack_size, filename_command_line_opt_pack_log_sh, true);
    }
}


#if 0
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
  file_command_line_dlv << "dlv -silent -filter=" << generalFilter <<" "<< TESTDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq "<< std::endl;
  file_command_line_dlv << "dlv -silent -filter=" << generalFilter <<" "<< TESTDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq | wc -l"<< std::endl;

  file_command_line_dlv.close();
}


void
print_opt_dlv_command_lines()
{
  std::string filename_command_line_dlv_opt  = prefix + OPT_DLV_CMD_EXT;
  file_command_line_dlv_opt.open(filename_command_line_dlv_opt.c_str());

  std::string optimumFilter = getOptimumDLVFilter();
  file_command_line_dlv_opt << "dlv -silent -filter=" << optimumFilter <<" "<< TESTDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq "<< std::endl;
  file_command_line_dlv_opt << "dlv -silent -filter=" << optimumFilter <<" "<< TESTDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq | wc -l"<< std::endl;

  file_command_line_dlv_opt.close();
}
#endif

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

  if (gen_data)
    {
      DMCS_LOG_TRACE("generate_orig_topology");
      generate_orig_topology();

      DMCS_LOG_TRACE("generate_contexts");
      generate_contexts();
 
      DMCS_LOG_TRACE("print_command_lines");
      print_command_lines();

      if (topology_type != RANDOM_TOPOLOGY && topology_type != DIAMOND_ARBITRARY_TOPOLOGY &&
	  topology_type != RING_EDGE_TOPOLOGY && topology_type != BINARY_TREE_TOPOLOGY)
	{
	  generate_opt_topology();
	  write_plans(true);
	}
      else
	{
	  write_plans(false);
	}
    }
  else
    {
      DMCS_LOG_TRACE("print_command_lines ONLY");
      print_command_lines();
    }


  /*
  //DMCS_LOG_TRACE("print_dlv_command_lines");
  //print_dlv_command_lines();

  // binary tree topology is a special case, orig_qp can be reused to
  // print opt_qp. Hence, we just need to print without any further
  // construction for the opt case.
  if (topology_type == BINARY_TREE_TOPOLOGY)
    {
      print_query_plan(orig_qp, prefix + OPT_EXT);
      print_opt_command_lines();
      //print_opt_dlv_command_lines();
    }

  // only for some fixed topologies where optimization is possible

  */

  return 0;
}
