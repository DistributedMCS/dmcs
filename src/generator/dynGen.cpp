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
 * @file   dynGen.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jun  29 13:22:00 2010
 * 
 * @brief  For the moment, we do hardcode generation for the student example.
 * 
 * 
 */


#include "generator/DynGridTopoGenerator.h"
#include "generator/DynRandomTopoGenerator.h"
#include "generator/DynRakeTopoGenerator.h"
#include "generator/DynSocialTopoGenerator.h"

#include "dmcs/Log.h"

#include <boost/program_options.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <sys/time.h>

using namespace dmcs::generator;

#define BR_EXT  "br"
#define CMD_EXT "cmd"
#define INP_EXT "inp"
#define DOT_EXT "dot"
#define SH_EXT  "sh"
#define TIME_EXT "tim"

#define DMCSD "dmcsd"
#define DMCSC "dmcsc"
#define TESTSDIR "."
#define LOCALHOST "localhost"
#define LIMIT_ANSWERS "n"
#define LIMIT_BIND_RULES "b"
#define HEURISTICS "h"
#define ROWS "rows"
#define COLS "cols"

#define POOLSIZE "PoolSize = "
#define CONTEXTS_INFO_TAG "[ContextsInfo]"
#define CONTEXT "C"
#define SIGNATURES_TAG "[Signature]"
#define MATCH_MAKER_TAG "[MM]"
#define MATCHES "matches = "
#define NOSBAS "nosba"


#define BRIDGE_RULE1 "inc :- (X:[pos]), not (Y:[neg]), not "
#define BRIDGE_RULE2 "dec :- (X:[neg]), not (Y:[pos]), not "
#define BRIDGE_RULE3 "inc v dec :- (X:[pos]), (X:[neg])."

#define HELP     "help"
#define CONTEXTS "poolsize"
#define PREFIX   "prefix"
#define TOPOLOGY "topology"
#define SLEEP    "sleep"
#define LOG      "log"
#define DENSITY  "d"

#define TESTSPATH "$TESTSPATH"
#define DMCSPATH  "$DMCSPATH"
#define RESPATH  "$RESPATH"

#define HELP_MESSAGE_TOPO "Choose topology for the initial dynamic system\n\
    0: Random Topology\n\
    1: Rake   Topology\n\
    2: Grid   Topology\n\
    3: Social Topology"

#define RANDOM_TOPO 0
#define RAKE_TOPO   1
#define GRID_TOPO   2
#define SOCIAL_TOPO 3

std::size_t poolsize;
std::string filename;
std::string prefix;
std::ofstream file_local_kb;
std::ofstream file_bridge_rules;
std::ofstream file_match_maker;
std::ofstream file_command_lines;
std::ofstream file_script;
std::ofstream file_topo;

std::vector<std::string> local_atoms;
std::vector<std::string> signature;
std::vector<std::size_t> no_bridge_rules;

BridgeRuleVecPtr pattern_bridge_rules(new BridgeRuleVec);
IntVecPtr no_sbridge_atoms(new IntVec);

std::size_t limit_answers;
std::size_t limit_bind_rules;
std::size_t topology;
std::size_t flipping;
std::size_t no_heuristics;
std::size_t time_sleep;
std::size_t density;
std::size_t m;
std::size_t n;

bool want_log;

// the bigger the test cases, the more time needed to load initial
// information from the (for now the text file), hence the shell needs
// to sleep a bit before calling dmcsc; otherwise, we always get
// "Connection refused"

std::string contexts_info;
std::string signatures;
std::string mm;
std::string nosbas;

MatchVecPtr mt(new MatchVec);
GraphPtr initial_topology(new Graph);
BridgeRuleVecVecPtr bridge_rules(new BridgeRuleVecVec);

DynTopologyGenerator* generator;

std::size_t
read_input(int argc, char* argv[])
{
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    (HELP, "Help message")
    (CONTEXTS, boost::program_options::value<std::size_t>(&poolsize)->default_value(3), "Contexts pool size")
    (PREFIX, boost::program_options::value<std::string>(&prefix)->default_value("student"), "Set up prefix for all files ")
    (LIMIT_ANSWERS, boost::program_options::value<std::size_t>(&limit_answers)->default_value(3), "Set up limit of answers ")
    (LIMIT_BIND_RULES, boost::program_options::value<std::size_t>(&limit_bind_rules)->default_value(8), "Set up limit of binding rules ")
    (TOPOLOGY, boost::program_options::value<std::size_t>(&topology)->default_value(0), HELP_MESSAGE_TOPO)
    (HEURISTICS, boost::program_options::value<std::size_t>(&no_heuristics)->default_value(5), "Set number of heuristics ")
    (SLEEP, boost::program_options::value<std::size_t>(&time_sleep)->default_value(2), "Sleeping time before call dmcsc ")
    (LOG, boost::program_options::value<bool>(&want_log)->default_value(false), "Specify whether we want log information ")
    (DENSITY, boost::program_options::value<std::size_t>(&density)->default_value(3), "Specify the density of the random topology. Value should be in [1,10] ")
    (ROWS, boost::program_options::value<std::size_t>(&m)->default_value(3), "Number of rows, for grid topology only ")
    (COLS, boost::program_options::value<std::size_t>(&n)->default_value(5), "Number of columns, for grid topology only ")
    ;
  
  boost::program_options::variables_map vm;        
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count(HELP))
    {
      std::cerr << desc << std::endl;
      return 1;
    }
  return 0;
}


void
init()
{
  // this is a very bad idea, it will only change the seed every second!
  // srand( time(NULL) );
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + tv.tv_usec);
  }

  // pos and neg from other contexts can be pointed to one of these
  local_atoms.push_back("s");
  local_atoms.push_back("h");
  local_atoms.push_back("e");

  // for now we just have three schematic bridge rules
  pattern_bridge_rules->push_back(BRIDGE_RULE1);
  pattern_bridge_rules->push_back(BRIDGE_RULE2);
  pattern_bridge_rules->push_back(BRIDGE_RULE3);
  
  contexts_info = CONTEXTS_INFO_TAG;
  contexts_info = contexts_info + "\n";

  signatures = SIGNATURES_TAG ;
  signatures = signatures + "\n";

  mm = MATCH_MAKER_TAG;
  mm = mm + "\n";

  nosbas = NOSBAS;
  nosbas = nosbas + " = ";

  if (topology == GRID_TOPO)
    {
      poolsize = m*n;
    }

  no_sbridge_atoms->assign(poolsize, 0);

  signature.push_back("s");
  signature.push_back("h");
  signature.push_back("e");
  signature.push_back("ps");
  signature.push_back("ph");
  signature.push_back("pe");
  signature.push_back("inc");
  signature.push_back("dec");
  signature.push_back("interesting");
  signature.push_back("hard_prof");
  signature.push_back("pos");
  signature.push_back("neg");

  for (std::size_t i = 0; i < poolsize; ++i)
    {
      boost::add_vertex(*initial_topology);

      BridgeRuleVecPtr brs(new BridgeRuleVec);
      bridge_rules->push_back(brs);
    }
}


void
write_bridge_rules_file()
{
  std::stringstream out;

  for (std::size_t i = 1; i <= poolsize; ++i)
    {
      out.str("");
      out << i;

      filename = prefix;
      filename = filename + "-" + out.str() + "." + BR_EXT;
      
      file_bridge_rules.open(filename.c_str());
      
      const BridgeRuleVecPtr brv = (*bridge_rules)[i-1];

      for (BridgeRuleVec::const_iterator it = brv->begin(); it != brv->end(); ++it)
	{
	  file_bridge_rules << *it;
	}

      file_bridge_rules.close();
    }
}


void
write_match_maker_file()
{
  filename = prefix;
  filename = filename + "." + INP_EXT;
  file_match_maker.open(filename.c_str());

  file_match_maker << POOLSIZE << poolsize << std::endl << std::endl;
  file_match_maker << contexts_info << std::endl << std::endl;
  file_match_maker << signatures << std::endl << std::endl;
  file_match_maker << MATCH_MAKER_TAG << std::endl << MATCHES;

  MatchVec::const_iterator beg = mt->begin();
  MatchVec::const_iterator end = mt->end();
  --end;

  std::copy(beg, end, std::ostream_iterator<Match>(file_match_maker, ", "));

  file_match_maker << *end << std::endl << std::endl;

  std::stringstream out;
  for (std::vector<std::size_t>::const_iterator it = no_sbridge_atoms->begin();
       it != no_sbridge_atoms->end(); ++it)
    {
      out.str("");
      out << *it;
      nosbas = nosbas + out.str() + " ";
    }

  file_match_maker << nosbas << std::endl;
  file_match_maker.close();
}

void
generate_command_lines(std::size_t heuristics)
{
  std::stringstream out;
  out << heuristics;

  filename = prefix;
  filename = filename + out.str() + "." + CMD_EXT;

  std::string filename_sh = prefix;
  filename_sh = filename_sh + out.str() + "." + SH_EXT;

  std::string filename_time = prefix;
  filename_time = filename_time + out.str() + "." + TIME_EXT;
  
  file_command_lines.open(filename.c_str());
  file_script.open(filename_sh.c_str());

  file_script << "#!/bin/bash" << std::endl
	      << "export TESTSPATH=\'.\'" << std::endl
	      << "export DMCSPATH=\'../../build/src\'" << std::endl
	      << "export RESPATH=\'./result\'" << std::endl;

  file_script << "killall dmcsd\n";

  for (std::size_t i = 1; i <= poolsize; ++i)
    {
      file_command_lines << "../../build-dbg/src/dmcsd --context=" << i << " --port=" << 5000+i 
			 << " --kb=./" << prefix << "-" << i 
			 << ".lp --br=./" << prefix << "-" << i 
			 << ".br --dynamic=1 --mm=./" << prefix << ".inp --n=" 
			 << limit_answers << " --b=" << limit_bind_rules << " --h=" << out.str() << "\n";

      file_script << "$DMCSPATH/dmcsd --context=" << i << " --port=" << 5000+i 
		  << " --kb=$TESTSPATH/" << prefix << "-" << i
		  << ".lp --br=$TESTSPATH/" << prefix << "-" << i
		  << ".br --dynamic=1 --mm=$TESTSPATH/" << prefix << ".inp --n=" << limit_answers << " --b=" << limit_bind_rules 
		  << " --h=" << out.str();
      
      if (want_log)
	{
	  file_script << " > " << prefix << "-" << i << ".log 2>&1 &\n";
	}
      else
	{
	  file_script << " >/dev/null 2>&1 &\n";
	}
    }

  file_command_lines << "gtime --verbose --o ./result/" << filename_time << " ../../build-dbg/src/dmcsc --hostname=localhost --port=5001 --dynamic=1\n";

  out.str("");
  out << time_sleep;

  // need to sleep for some second for all contexts to start up
  file_script << "sleep " << out.str() << std::endl;
  //  file_script << "$DMCSPATH/dmcsc --hostname=localhost --port=5001 --dynamic=1\n";

  file_script << "gtime --verbose --o " << " $RESPATH/" << filename_time << " $DMCSPATH/dmcsc --hostname=localhost --port=5001 --dynamic=1\n";
  file_script << "killall dmcsd\n";

  file_command_lines.close();
  file_script.close();
}


void
generate_local_kb(std::size_t i)
{
  std::stringstream out;
  out << i;
  std::string filename_local_kb = prefix + "-" + out.str() + ".lp";
 
  file_local_kb.open(filename_local_kb.c_str());
  
  // The basic rules
  file_local_kb << "s :- ps, not h, not e." << std::endl;
  file_local_kb << "s :- ph, not ps, not h, not e, inc." << std::endl;
  file_local_kb << "h :- ps, not e, dec." << std::endl;
  file_local_kb << "h :- ph, not inc, not dec." << std::endl;
  file_local_kb << "h :- pe, not ph, inc." << std::endl;
  file_local_kb << "e :- pe, not h, not inc." << std::endl;
  file_local_kb << "e :- ph, dec." << std::endl;
  file_local_kb << "ps :- interesting." << std::endl;
  file_local_kb << "pe :- hard_prof." << std::endl;
  file_local_kb << "ph :- ps, pe." << std::endl;
  file_local_kb << "ph :- not ps, not pe." << std::endl;

  // And then optional facts
  std::size_t choose_fact = rand() % 4;
  switch (choose_fact)
    {
    case 0:
      {
	file_local_kb << "interesting." << std::endl;
	break;
      }
    case 1:
      {
	file_local_kb << "hard_prof." << std::endl;
	break;
      }
    case 2:
      {
	file_local_kb << "interesting." << std::endl;
	file_local_kb << "hard_prof." << std::endl;
	break;
      }
    }

  file_local_kb.close();
}


int main(int argc, char* argv[])
{
  if (read_input(argc, argv))
    {
      return 1;
    }

  init(); 

  for (std::size_t i = 1; i <= poolsize; ++i)
    {
      std::stringstream id;
      std::stringstream port;
      id << i;
      
      port << 5000 + i;
      contexts_info = contexts_info + CONTEXT + id.str() + " = " + id.str() + " localhost " + port.str() + "\n";
      signatures = signatures + CONTEXT + id.str() + " = ";

      // generate signatures for context i
      std::stringstream lid;
      std::size_t end = signature.size() - 1;
      for (std::size_t j = 0; j < end; ++j)
	{
	  lid.str("");
	  lid << j+1;
	  signatures = signatures + "(" + signature[j] + " " + id.str() + " " + lid.str() + " " + lid.str() + "), ";
	}

      lid.str("");
      lid << end + 1;
      signatures = signatures + "(" + signature[end] + " " + id.str() + " " + lid.str() + " " + lid.str() + ")\n";      

      generate_local_kb(i);
    }

  dmcs::init_loggers("dynGen");

  DMCS_LOG_INFO("poolsize: " << poolsize);
  
  // Topology choosing
  switch (topology)
    {
    case RANDOM_TOPO:
      {
	generator = new DynRandomTopoGenerator(mt, initial_topology, bridge_rules, 
					       pattern_bridge_rules, no_sbridge_atoms, 
					       density, poolsize);
	break;
      }
    case RAKE_TOPO:
      {
	generator = new DynRakeTopoGenerator(mt, initial_topology, bridge_rules, 
					     pattern_bridge_rules, no_sbridge_atoms, 
					     poolsize);
      break;
      }
    case GRID_TOPO:
      {
	generator = new DynGridTopoGenerator(mt, initial_topology, bridge_rules, 
					     pattern_bridge_rules, no_sbridge_atoms, 
					     m, n);
	break;
      }
    case SOCIAL_TOPO:
      {
	generator = new DynSocialTopoGenerator(mt, initial_topology, bridge_rules, 
					       pattern_bridge_rules, no_sbridge_atoms, 
					       poolsize);
	break;
      }
    }
  
  generator->generate_dynamic_system();
  write_bridge_rules_file();
  write_match_maker_file();

  for (std::size_t i = 0; i <= no_heuristics; ++i)
    {
      generate_command_lines(i);
    }
  
  filename = prefix;
  filename = filename + "." + DOT_EXT;
  file_topo.open(filename.c_str());
  boost::write_graphviz(file_topo, *initial_topology);
  file_topo.close();
}


// Local Variables:
// mode: C++
// End:
