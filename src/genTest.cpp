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
 * @file   genTest.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  18 16:14:00 2010
 * 
 * @brief  
 * 
 * 
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Context.h"
#include "ContextGenerator.h"
#include "BinaryTreeQPGenerator.h"
#include "DiamondQPGenerator.h"
#include "DiamondOptQPGenerator.h"
#include "DiamondZigZagQPGenerator.h"
#include "DiamondZigZagOptQPGenerator.h"
#include "HouseQPGenerator.h"
#include "HouseOptQPGenerator.h"
#include "LogicVisitor.h"
#include "DLVVisitor.h"
#include "MultipleRingQPGenerator.h"
#include "MultipleRingOptQPGenerator.h"
#include "RingQPGenerator.h"
#include "RingEdgeQPGenerator.h"
#include "RingOptQPGenerator.h"
#include "QueryPlan.h"
#include "QueryPlanGenerator.h"
#include "Rule.h"
#include "Signature.h"
#include "ProgramOptions.h"

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
#define OPT_DLV_CMD_EXT "_opt_dlv.txt"

ContextsPtr contexts(new Contexts);

std::vector<Signature> sigmas;

std::size_t no_contexts;
int no_atoms;
int no_interface_atoms;
int no_bridge_rules;
int topology_type;

std::string filename;
std::ofstream file_rules;
std::ofstream file_topo;
std::ofstream file_command_line;
std::ofstream file_command_line_sh;
std::ofstream file_command_line_opt;
std::ofstream file_command_line_opt_sh;
std::ofstream file_dlv;
std::ofstream file_command_line_dlv;
std::ofstream file_command_line_opt_dlv;

QueryPlanGenerator* qpgen;
QueryPlanGenerator* opt_qpgen;

QueryPlanPtr original_qp;
QueryPlanPtr optimal_qp;

int
read_input(int argc, char* argv[])
{
  if (argc != 7)
    {
      std::cerr << "Usage: " << argv[0] << " <no_contexts> <no_atoms> <no_interface_atoms> <no_bridge_rules> <topology_type> <filename>";
      std::cerr << std::endl;
      std::cerr << std::endl;
      std::cerr << "       <topology_type> = 0 : random topology" << std::endl;
      std::cerr << "       <topology_type> = 1 : chain of diamonds topology (all way down)" << std::endl;
      std::cerr << "       <topology_type> = 2 : chain of diamonds topology (arbitrary edges)" << std::endl;
      std::cerr << "       <topology_type> = 3 : chain of zig-zag diamonds topology" << std::endl;
      std::cerr << "       <topology_type> = 4 : pure ring topology" << std::endl;
      std::cerr << "       <topology_type> = 5 : ring topology (with additional edges)" << std::endl;
      std::cerr << "       <topology_type> = 6 : binary tree topology" << std::endl;
      std::cerr << "       <topology_type> = 7 : house topology" << std::endl;
      std::cerr << "       <topology_type> = 8 : multiple ring topology" << std::endl;
      std::cerr << std::endl;
      return 1;
    }

  no_contexts = std::atoi(argv[1]);
  no_atoms = std::atoi(argv[2]);
  no_interface_atoms = std::atoi(argv[3]);
  no_bridge_rules = std::atoi(argv[4]);
  topology_type = std::atoi(argv[5]);

  filename = argv[6];

  if ((1 <= topology_type) && (topology_type <= 3) && (no_contexts % 3 != 1))
    {
      std::cerr << "For \"chain of diamond\" topology, the number of contexts must be 3n+1" 
		<< std::endl;
      return 1;
    }
  if ((topology_type == 7) && (no_contexts % 4 != 1))
    {
      std::cerr << "For \"house\" topology, the number of contexts must be 4n+1" << std::endl;
      return 1;
    }
  if ((topology_type == 8)&& (no_contexts < 6) && (no_contexts % 3 != 0) )
    {
      std::cerr << "For \"multiple ring\" topology, the number of contexts must be 3(n-1) where n >= 3" 
		<< std::endl;
      return 1;
    }


  std::cerr << "Number of context:                             " << no_contexts << std::endl;
  std::cerr << "Number of atoms per context:                   " << no_atoms << std::endl;
  std::cerr << "Number of maximum interface atoms per context: " << no_interface_atoms << std::endl;
  std::cerr << "Number of maximum bridge rules per context:    " << no_bridge_rules << std::endl;
  std::cerr << "Topology type:                                 " << topology_type << std::endl;
  std::cerr << "Prefix for filename:                           " << filename << std::endl;
  std::cerr << std::endl;

  return 0;
}

void
init()
{
  std::stringstream out;
  std::string atom_name;
  std::string local_sigma;


  QueryPlanPtr tmp1(new QueryPlan);
  original_qp = tmp1;

  QueryPlanPtr tmp2(new QueryPlan);
  optimal_qp = tmp2;

  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      local_sigma = "";
      out.str("");
      out << i;

      SignaturePtr s(new Signature);

      // create local signature for context i
      for (int j = 0; j < no_atoms; ++j)
	{
	  atom_name = (char)(j+'a') + out.str();
	  s->insert(Symbol(atom_name, i, j+1, j+1));
	  local_sigma = local_sigma + atom_name;
	  if (j < no_atoms - 1)
	    {
	      local_sigma = local_sigma + " ";
	    }
	}

      sigmas.push_back(*s);

      RulesPtr r(new Rules);
      BridgeRulesPtr br(new BridgeRules);
      ContextPtr c(new Context(i, no_contexts, s, optimal_qp, r, br));

      contexts->push_back(c);
    }
}


void
setupInfo()
{
  std::stringstream out;
  std::string str_port;

  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      out.str("");
      out << 5000 + i;
      str_port = out.str();

      out.str("");
      out << i;

      //      std::cerr << i << std::endl;
      original_qp->putSignature(i, sigmas[i-1]);
      original_qp->putHostname(i, STR_LOCALHOST);
      original_qp->putPort(i, str_port);
    }
}


void generate_topology()
{
  switch (topology_type)
    {
    case 1:
      qpgen = new DiamondQPGenerator(contexts, original_qp);
      opt_qpgen = new DiamondOptQPGenerator(contexts, optimal_qp);
      break;
    case 3:
      qpgen = new DiamondZigZagQPGenerator(contexts, original_qp);
      opt_qpgen = new DiamondZigZagOptQPGenerator(contexts, optimal_qp);
      break;
    case 4:
      qpgen = new RingQPGenerator(contexts, original_qp);
      opt_qpgen = new RingOptQPGenerator(contexts, optimal_qp);
      break;
    case 5:
      ///@todo Either create the extra edges statically 
      ///      or dynamically find the optimium query plan
      qpgen = new RingEdgeQPGenerator(contexts, original_qp);
      break;
    case 6:
      qpgen = new BinaryTreeQPGenerator(contexts, original_qp);
      opt_qpgen = new BinaryTreeQPGenerator(contexts, optimal_qp);
      break;
    case 7:
      qpgen = new HouseQPGenerator(contexts, original_qp);
      opt_qpgen = new HouseOptQPGenerator(contexts, optimal_qp);
      break;
    case 8:
      qpgen = new MultipleRingQPGenerator(contexts, original_qp);
      opt_qpgen = new MultipleRingOptQPGenerator(contexts, optimal_qp);
      break;
    }
  qpgen->generate();
  setupInfo();
}


std::string
interface_vars(ContextPtr& context, std::size_t neighbor_id)
{
  std::string temp = "\"";
  std::stringstream out;
  out << neighbor_id;

  const BridgeRulesPtr br = context->getBridgeRules();

  for (BridgeRules::const_iterator i = br->begin(); i != br->end(); ++i)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*i);
      const NegativeBridgeBody& nb = getNegativeBody(*i);

      for (PositiveBridgeBody::const_iterator j = pb.begin(); j != pb.end(); ++j)
	{
	  if (j->first == neighbor_id)
	    {
	      temp = temp + (char)(j->second + 96) + out.str() + " ";
	    }
	}

      for (NegativeBridgeBody::const_iterator j = nb.begin(); j != nb.end(); ++j)
	{
	  if (j->first == neighbor_id)
	    {
	      temp = temp + (char)(j->second + 96) + out.str() + " ";
	    }
	}
    }

  temp = temp + "\"";
  return temp;
}

void
print_contexts()
{
  std::ostringstream oss;
  LogicVisitor lv(oss);
  std::ostringstream ossDLV;
  DLVVisitor dv(ossDLV);
  std::stringstream out;

  for (Contexts::const_iterator i = contexts->begin(); i != contexts->end(); ++i)
    {
      out.str("");
      out << (*i)->getContextID();
      std::string filename_lp = filename + "-" + out.str() + LP_EXT;
      std::string filename_br = filename + "-" + out.str() + BR_EXT;
      
      file_rules.open(filename_lp.c_str());
      lv.visitRules(*i);
      dv.visitRules(*i);
      file_rules << oss.str();
      oss.str("");
      file_rules.close();
       
      file_rules.open(filename_br.c_str());
      lv.visitBridgeRules(*i);
      dv.visitBridgeRules(*i);
      file_rules << oss.str();
      oss.str("");
      file_rules.close();
    }
  std::string filename_dlv = filename + DLV_EXT;

  file_dlv.open(filename_dlv.c_str());
  file_dlv << ossDLV.str();
  file_dlv.close();

}

void
print_topo()
{
  std::string filename_topo     = filename + TOP_EXT;
  std::string filename_topo_opt = filename + OPT_EXT;

  file_topo.open(filename_topo.c_str());
  original_qp->write_graph(file_topo);
  file_topo.close();

  file_topo.open(filename_topo_opt.c_str());
  optimal_qp->write_graph(file_topo);
  file_topo.close();
}


//DLV file:
//loop in all contexts and write to file

// optimal DLV:
//get neighbours, or them together


// orignial DLV:
// get vars from sigmas, except for sigma 1
//get everything from sigma 1

const std::string
getOptimumDLVFilter() 
{
  std::string result = "";
  NeighborsPtr_ neighbors =optimal_qp->getNeighbors1(1);
  BeliefState neighborsInterface(no_contexts);
  
  for(Neighbors_::const_iterator it = neighbors->begin(); it != neighbors->end();++it )
    {
      const BeliefStatePtr& currentInterface=  optimal_qp->getInterface1(1, *it);
      BeliefSets::iterator jt = currentInterface.belief_state_ptr->belief_state.begin();
      BeliefSets::iterator kt = neighborsInterface.belief_state.begin();

      for (; jt != currentInterface.belief_state_ptr->belief_state.end(); ++jt,++kt)
	{
	  *kt |= *jt;
	}
      
    }
    BeliefSets::iterator kt = neighborsInterface.belief_state.begin();
    *kt |= std::numeric_limits<unsigned long>::max();
  
  ///@todo define constant names for topologies types
    if (topology_type == 1)
    {
      const BeliefStatePtr& currentInterface =  optimal_qp->getInterface1(2, 4);
      BeliefSets::iterator jt = currentInterface.belief_state_ptr->belief_state.begin();
      BeliefSets::iterator kt = neighborsInterface.belief_state.begin();

      for (; jt != currentInterface.belief_state_ptr->belief_state.end(); ++jt,++kt)
	{
	  *kt |= *jt;
	}

      const BeliefStatePtr& currentInterface1 =  optimal_qp->getInterface1(3, 4);
      BeliefSets::iterator jt1 = currentInterface1.belief_state_ptr->belief_state.begin();
      BeliefSets::iterator kt1 = neighborsInterface.belief_state.begin();
      
      for (; jt1 != currentInterface1.belief_state_ptr->belief_state.end(); ++jt1,++kt1)
	{
	  *kt1 |= *jt1;
	}

    }

    if (topology_type == 3)
      {
	const BeliefStatePtr& currentInterface1 =  optimal_qp->getInterface1(2, 3);
	BeliefSets::iterator jt1 = currentInterface1.belief_state_ptr->belief_state.begin();
	BeliefSets::iterator kt1 = neighborsInterface.belief_state.begin();
	
	for (; jt1 != currentInterface1.belief_state_ptr->belief_state.end(); ++jt1,++kt1)
	  {
	    *kt1 |= *jt1;
	  }
      }


    bool first = true;
    std::size_t contextID = 1;
    for(;contextID <= no_contexts; ++contextID)
      {
	const Signature& contextSignature  = optimal_qp->getSignature1(contextID);
      
	const SignatureByLocal& contextSigByLocal = boost::get<Tag::Local>(contextSignature);
	BeliefSet contextInterface = neighborsInterface.belief_state[contextID-1];
	SignatureByLocal::const_iterator context_it; 
	
	if (!isEpsilon(contextInterface))
	  {
	    std::size_t i = 1; // ignore epsilon bit
	    
	  for (; i < sizeof(contextInterface)*8 ; ++i)
	    {
	      if ((contextInterface  & (1 << i)) && i <= contextSignature.size())
		{
		  context_it = contextSigByLocal.find(i); 
		  assert(context_it != contextSigByLocal.end());
		  if(!first)
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


const std::string
getGeneralDLVFilter()
{
  const BeliefStatePtr& globalInterface = original_qp->getGlobalV();
  std::string result = ""; 
  bool first = true;
  std::size_t contextID = 1;
  for(;contextID <= no_contexts; ++contextID)
    {
      const Signature& contextSignature  = original_qp->getSignature1(contextID);
      const SignatureByLocal& contextSigByLocal = boost::get<Tag::Local>(contextSignature);
      BeliefSet contextInterface = globalInterface.belief_state_ptr->belief_state[contextID-1];
      SignatureByLocal::const_iterator context_it; 

      if (!isEpsilon(contextInterface))
	{
	  std::size_t i = 1; // ignore epsilon bit

	  for (; i < sizeof(contextInterface)*8 ; ++i)
	    {
	      if(contextID != 1)
		{
		  if (contextInterface  & (1 << i))
		    {
		      context_it = contextSigByLocal.find(i); 
		      assert(context_it != contextSigByLocal.end());
		      if(!first)
			{
			  result += ",";
			}
		      else 
			{
			  first = false;
			}
		      result += context_it->sym;
		    }
		}// this extra if is used to make sure that all of sigma 1 is used, regardless of global V
	      else if(i <= contextSignature.size())
		{
		  context_it = contextSigByLocal.find(i); 
		  assert(context_it != contextSigByLocal.end());
		  if(!first)
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
printDLVCommmandLines()
{
  std::string filename_command_line_dlv      = filename + DLV_CMD_EXT;
  std::string filename_command_line_opt_dlv  = filename + OPT_DLV_CMD_EXT;

  file_command_line_dlv.open(filename_command_line_dlv.c_str());
  file_command_line_opt_dlv.open(filename_command_line_opt_dlv.c_str());

  std::string generalFilter = getGeneralDLVFilter();
  file_command_line_dlv << "dlv -silent -filter=" << generalFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq "<< std::endl;
  file_command_line_dlv << "dlv -silent -filter=" << generalFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq | wc -l"<< std::endl;

  std::string optimumFilter = getOptimumDLVFilter();
  file_command_line_opt_dlv << "dlv -silent -filter=" << optimumFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq "<< std::endl;
  file_command_line_opt_dlv << "dlv -silent -filter=" << optimumFilter <<" "<< TESTSDIR <<"/"<< filename << DLV_EXT<<" | sort | uniq | wc -l"<< std::endl;


  file_command_line_dlv.close();
  file_command_line_opt_dlv.close();
}

void
print_command_lines()
{
  printDLVCommmandLines();

  std::string filename_command_line        = filename + CMD_EXT;
  std::string filename_command_line_sh     = filename + SH_CMD_EXT;
  std::string filename_command_line_opt    = filename + OPT_CMD_EXT;
  std::string filename_command_line_opt_sh = filename + OPT_SH_CMD_EXT;
 

  file_command_line.open(filename_command_line.c_str());
  file_command_line_sh.open(filename_command_line_sh.c_str());
  file_command_line_opt.open(filename_command_line_opt.c_str());
  file_command_line_opt_sh.open(filename_command_line_opt_sh.c_str());


  file_command_line_sh << "#!/bin/bash" << std::endl
		       << "export TIMEFORMAT=$'\\nreal\\t%3R\\nuser\\t%3U\\nsys\\t%3S'" << std::endl
		       << "export TESTSPATH='" TESTSDIR "'" << std::endl
		       << "export DMCSPATH='.'" << std::endl;

  file_command_line_opt_sh << "#!/bin/bash" << std::endl
			   << "export TIMEFORMAT=$'\\nreal\\t%3R\\nuser\\t%3U\\nsys\\t%3S'" << std::endl
			   << "export TESTSPATH='" TESTSDIR "'" << std::endl
			   << "export DMCSPATH='.'" << std::endl;


  // dmcsd commands
  // ./dmcsd <id> <hostname> <port> <filename_lp> <filename_br> <filename_topo> 

  std::string command_line_opt_sh;
  std::string command_line_opt;
  std::string command_line_sh;
  std::string command_line;

  // for each C_i
  for (Contexts::iterator i = contexts->begin(); i != contexts->end(); ++i)
    {
      std::stringstream out;
      std::stringstream index;
      ContextPtr context = *i;

      index << context->getContextID();

      // ID PortNo@Hostname
      out << "--" << CONTEXT_ID << "=" << context->getContextID() << " " 
	//	  << original_qp->getHostname1(context->getContextID()) << " "
	  << "--" << PORT << "=" << original_qp->getPort1(context->getContextID());

      command_line_opt_sh = "$DMCSPATH/" DMCSD " " + out.str() + 
	" --" KB "=$TESTSPATH/" + filename + "-" + index.str() + LP_EXT +
	" --" BR "=$TESTSPATH/" + filename + "-" + index.str() + BR_EXT +
	" --"TOPOLOGY "=$TESTSPATH/" + filename + OPT_EXT;

      command_line_sh = "$DMCSPATH/" DMCSD " " + out.str() + 
	" --" KB "=$TESTSPATH/" + filename + "-" + index.str() + LP_EXT +
	" --" BR "=$TESTSPATH/" + filename + "-" + index.str() + BR_EXT +
	" --" TOPOLOGY "=$TESTSPATH/" + filename + TOP_EXT;

      command_line_opt = "./" DMCSD " " + out.str() + 
	" --" KB "=" TESTSDIR "/" + filename + "-" + index.str() + LP_EXT +
	" --" BR "=" TESTSDIR "/" + filename + "-" + index.str() + BR_EXT +
	" --" TOPOLOGY "=" TESTSDIR "/" + filename + OPT_EXT;

      command_line = "./" DMCSD " " + out.str() + 
	" --" KB "=" TESTSDIR "/" + filename + "-" + index.str() + LP_EXT +
	" --" BR "=" TESTSDIR "/" + filename + "-" + index.str() + BR_EXT +
	" --" TOPOLOGY "=" TESTSDIR "/" + filename + TOP_EXT;

      file_command_line_opt_sh << command_line_opt_sh << " >/dev/null 2>&1 &" << std::endl;
      file_command_line_opt << command_line_opt << std::endl;
      file_command_line_sh << command_line_sh << " >/dev/null 2>&1 &" << std::endl;
      file_command_line << command_line << std::endl;
    }
  
  
  // client command
  // time | ./dmcsc localhost 5001

  ContextPtr context = *(contexts->begin());
  int start_id = context->getContextID();
  std::stringstream  globalV;
  std::stringstream  systemSize;
  globalV << original_qp->getGlobalV();
  systemSize << no_contexts;
  
  command_line_sh = "time $DMCSPATH/" DMCSC
    " --" HOSTNAME "=" + original_qp->getHostname1(start_id) +
    " --" PORT "=" + original_qp->getPort1(start_id) +
    " --" SYSTEM_SIZE "=" + systemSize.str() + 
    " --" QUERY_VARS "=\"" + globalV.str() + "\"";

  command_line = "time ./" DMCSC 
    " --" HOSTNAME "=" + original_qp->getHostname1(start_id) +
    " --" PORT "=" + original_qp->getPort1(start_id) +
    " --" SYSTEM_SIZE "=" + systemSize.str() + 
    " --" QUERY_VARS "=\"" + globalV.str() + "\"";

  command_line_opt_sh = "time $DMCSPATH/" DMCSC
    " --" HOSTNAME "=" + original_qp->getHostname1(start_id) +
    " --" PORT "=" + original_qp->getPort1(start_id) +
    " --" SYSTEM_SIZE "=" + systemSize.str();

  command_line_opt = "time ./"  DMCSC 
    " --" HOSTNAME "=" + original_qp->getHostname1(start_id) +
    " --" PORT "=" + original_qp->getPort1(start_id) +
    " --" SYSTEM_SIZE "=" + systemSize.str();

  file_command_line_sh << command_line_sh << std::endl << "killall " DMCSD << std::endl;
  file_command_line << command_line << std::endl;
  file_command_line_opt_sh << command_line_opt_sh << std::endl << "killall " DMCSD << std::endl;
  file_command_line_opt << command_line_opt << std::endl;


  file_command_line_sh.close();
  file_command_line.close();
  file_command_line_opt_sh.close();
  file_command_line_opt.close();
}

int main(int argc, char* argv[])
{
  if (read_input(argc, argv) == 1)
    { 
      return 1;
    }

  init();
  generate_topology();
  
  ContextGenerator context_gen(contexts, original_qp, no_bridge_rules, no_atoms);

  context_gen.generate();
  
  qpgen->create_interfaces();

  optimal_qp->putGraph(original_qp->getGraph());
  
  opt_qpgen->create_interfaces();
  
  print_contexts();
  print_topo();
  ///@todo update the command lines of invoking the client
  print_command_lines();
}

// Local Variables:
// mode: C++
// End:
