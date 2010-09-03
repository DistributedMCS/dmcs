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

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <boost/program_options.hpp>

#include "generator/dmcsGen.h"
#include "generator/DiamondTopoGenerator.h"
#include "generator/DiamondOptTopoGenerator.h"
#include "generator/ContextGenerator.h"
#include "ProgramOptions.h"

#include "Rule.h"
#include "Signature.h"

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
LocalInterfaceMapPtr opt_lcim;

TopologyGenerator* orig_topo_gen;
TopologyGenerator* opt_topo_gen;

NeighborVec2Ptr orig_topo(new NeighborVec2);
NeighborVec2Ptr opt_topo(new NeighborVec2);

std::size_t no_contexts;
std::size_t no_atoms;
std::size_t no_interface_atoms;
std::size_t no_bridge_rules;
std::size_t topology_type;

std::string prefix;
std::string filename;

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
    (PREFIX, boost::program_options::value<std::string>(&prefix)->default_value("student"), "Prefix for all files ")
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

  switch (topology_type)
    {
    case 1:
      { 
	if (no_contexts == 1 || no_contexts % 3 != 1)
	  {
	    std::cerr << "For \"Chain of diamond\" topology, the number of contexts must be 3n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case 2:
      { 
	if (no_contexts == 1 || no_contexts % 3 != 1)
	  {
	    std::cerr << "For \"Chain of diamond\" topology, the number of contexts must be 3n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case 3:
      { 
	if (no_contexts == 1 || no_contexts % 3 != 1)
	  {
	    std::cerr << "For \"Chain of zig-zag diamond\" topology, the number of contexts must be 3n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case 4:
      { 
	if (no_contexts == 1)
	  {
	    std::cerr << "For \"Pure Ring\" topology, the number of contexts must be at least 2." << std::endl;
	    return 1;
	  }
      }
      break;
    case 5:
      { 
	if (no_contexts == 1)
	  {
	    std::cerr << "For \"Ring\" topology, the number of contexts must be at least 2." << std::endl;
	    return 1;
	  }
      }
      break;
    case 6:
      { 
	if (no_contexts == 1)
	  {
	    std::cerr << "For \"binary tree\" topology, the number of contexts must be at least 2." << std::endl;
	    return 1;
	  }
      }
      break;
    case 7:
      { 
	if (no_contexts == 1 || no_contexts % 4 != 1)
	  {
	    std::cerr << "For \"House\" topology, the number of contexts must be 4n+1." << std::endl;
	    return 1;
	  }
      }
      break;
    case 8:
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

  std::cerr << "Number of context:                             " << no_contexts << std::endl;
  std::cerr << "Number of atoms per context:                   " << no_atoms << std::endl;
  std::cerr << "Number of maximum interface atoms per context: " << no_interface_atoms << std::endl;
  std::cerr << "Number of maximum bridge rules per context:    " << no_bridge_rules << std::endl;
  std::cerr << "Topology type:                                 " << topology_type << std::endl;
  std::cerr << "Prefix for filename:                           " << prefix << std::endl << std::endl;

  return 0;

}



// will incrementally write to these file streams for each context,
// and finally conclude with the dmcsc command
void
open_file_streams()
{
  std::string filename_command_line        = prefix + CMD_EXT;
  std::string filename_command_line_sh     = prefix + SH_CMD_EXT;
  std::string filename_command_line_opt    = prefix + OPT_CMD_EXT;
  std::string filename_command_line_opt_sh = prefix + OPT_SH_CMD_EXT;

  file_command_line.open(filename_command_line.c_str());
  file_command_line_sh.open(filename_command_line_sh.c_str());
  file_command_line_opt.open(filename_command_line_opt.c_str());
  file_command_line_opt_sh.open(filename_command_line_opt_sh.c_str());
}



void
close_file_streams()
{
  file_command_line_sh.close();
  file_command_line.close();
  file_command_line_opt_sh.close();
  file_command_line_opt.close();
}



void
setup_topos()
{
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
  open_file_streams();

  // initialize minV as we now know the system size
  BeliefStatePtr someV(new BeliefState(no_contexts, 0));
  minV = someV;

  // now set all epsilon bits to 1
  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      (*minV)[i] = setEpsilon((*minV)[i]);
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
    }

  orig_topo_gen->generate();

  #ifdef DEBUG
  std::cerr << "Original topology:" << std::endl;
  for (std::size_t i = 1; i <= no_contexts; ++ i)
    {
      std::cerr << i << " --> ";

      NeighborVecPtr neighbors = (*orig_topo)[i-1];
      std::copy(neighbors->begin(), neighbors->end(), std::ostream_iterator<std::size_t>(std::cerr, " "));

      std::cerr << std::endl;
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
  // edsges to loca interfaces, which will be used to compte the
  // interface in the optimal topology.

#ifdef DEBUG
  std::cerr << "minV = " << minV << std::endl;
  std::cerr << "Original local interface:" << std::endl;
  for (LocalInterfaceMap::const_iterator it = lcim->begin(); it != lcim->end(); ++it)
    {
      ContextPair cp = it->first;
      std::cerr << "(" << cp.first << ", " << cp.second << ") --> " 
		<< it->second << std::endl;
    }
#endif
}



void
generate_opt_topology()
{
  // Check whether we really copy lcim to opt_lcim
  LocalInterfaceMapPtr tmp(new LocalInterfaceMap(*lcim));
  opt_lcim = tmp;

  switch (topology_type)
    {
    case 1:
      {
	opt_topo_gen = new DiamondOptTopoGenerator(opt_topo, opt_lcim);
	break;
      }
    }

  opt_topo_gen->generate();

#ifdef DEBUG
  std::cerr << "OPTIMAL topology:" << std::endl;
  for (std::size_t i = 1; i <= no_contexts; ++ i)
    {
      std::cerr << i << " --> ";

      NeighborVecPtr neighbors = (*opt_topo)[i-1];
      std::copy(neighbors->begin(), neighbors->end(), std::ostream_iterator<std::size_t>(std::cerr, " "));

      std::cerr << std::endl;
    }
#endif

  opt_topo_gen->create_opt_interface();

#ifdef DEBUG
  #ifdef DEBUG
  std::cerr << "Optimal local interface:" << std::endl;
  for (LocalInterfaceMap::const_iterator it = opt_lcim->begin(); it != opt_lcim->end(); ++it)
    {
      ContextPair cp = it->first;
      std::cerr << "(" << cp.first << ", " << cp.second << ") --> " 
		<< it->second << std::endl;
    }
#endif
#endif
}



int 
main(int argc, char* argv[])
{
  if (read_input(argc, argv) == 1) // reading input not completed
    {
      return 0;
    }

  srand( time(NULL) );

  init();
  generate_orig_topology();
  generate_contexts();
  generate_opt_topology();


}
