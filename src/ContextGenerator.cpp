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
 * @file   ContextGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sat Jan  16 18:43:30 2010
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>
#include <sstream>
#include <fstream>

#include "LogicVisitor.h"
#include "generator/ContextGenerator.h"


namespace dmcs { namespace generator {

int
ContextGenerator::sign()
{
  return ( rand() % 2 )  ?  1  :  -1;
}



void
ContextGenerator::generate()
{
  std::size_t system_size = context_interfaces->size();

  for (std::size_t i = 1; i <= system_size; ++i)
    {
      local_kb->clear();
      bridge_rules->clear();

      generate_local_kb(i);
      
      NeighborListPtr neighbors = (*orig_topo)[i-1];
      if (neighbors->size() > 0)
	{
	  generate_bridge_rule_list(i);
	}

      write_local_kb(i);
      write_bridge_rules(i);
    }
}



void
ContextGenerator::generate_local_kb(std::size_t id)
{
  for (std::size_t i = 1; i <= no_atoms; ++i)
    {
      HeadPtr head(new Head);
      PositiveBodyPtr pbody(new PositiveBody);
      NegativeBodyPtr nbody(new NegativeBody);
      std::pair<PositiveBodyPtr, NegativeBodyPtr> body(pbody, nbody);
      RulePtr r(new Rule(head, body));

      head->push_back(i);
      if (i % 2 == 0)
	{
	  if (rand() % 2)
	    {
	      // going backward, always possible
	      nbody->push_back(i-1);
	      local_kb->push_back(r);
	    }
	  else
	    {
	      // going forward, need to check
	      if (i < no_atoms)
		{
		  nbody->push_back(i+1);
		  local_kb->push_back(r);
		}
	    }
	}
      else
	{
	  // going forward, need to check
	  if (i < no_atoms)
	    {
	      nbody->push_back(i+1);
	      local_kb->push_back(r);
	    }
	}
    }
}



std::size_t
addUniqueBridgeAtom(BridgeRulePtr& r, std::size_t neighbor_id, int neighbor_atom)
{
  BridgeAtom bap = std::make_pair(neighbor_id, std::max(neighbor_atom, -neighbor_atom));

#ifdef DEBUG
  std::cerr << "Adding atom " << neighbor_atom << " of neighbor " << neighbor_id << std::endl;
#endif
  PositiveBridgeBody& pb = getPositiveBody(r);
  NegativeBridgeBody& nb = getNegativeBody(r);

  if ((std::find(pb.begin(), pb.end(), bap) == pb.end()) &&
      (std::find(nb.begin(), nb.end(), bap) == nb.end()))
    {
      if (neighbor_atom > 0)
	{
	  pb.push_back(bap);
	}
      else
	{
	  nb.push_back(bap);
	}

      return 1;
    }

  return 0;
}



void
ContextGenerator::generate_bridge_rule(std::size_t id)
{
  HeadPtr head(new Head);
  PositiveBridgeBodyPtr pbody(new PositiveBridgeBody);
  NegativeBridgeBodyPtr nbody(new NegativeBridgeBody);
  std::pair<PositiveBridgeBodyPtr, NegativeBridgeBodyPtr> body(pbody, nbody);
  BridgeRulePtr r(new BridgeRule(head, body));

  std::size_t br_h = (rand() % no_atoms) + 1;
  head->push_back(br_h);

  // either 1 or 2 bridge atom(s)
  int no_bridge_atoms = (rand() % 2) + 1; 

  // loop until enough unique atoms
  std::size_t count = 0;

  NeighborListPtr nbors = (*orig_topo)[id-1];

  do {
    std::size_t nbors_size = nbors->size();

    std::cerr << id << " ---> " << nbors_size << std::endl;

    std::size_t nbor_pos  = (rand() % nbors_size);
    std::size_t nbor_id   = (*nbors)[nbor_pos];

    Interface nbor_interface = (*context_interfaces)[nbor_id - 1];
    
    std::size_t no_interface_atoms = nbor_interface.size();
    std::size_t atom_pos = rand() % no_interface_atoms;
    int atom = sign() * nbor_interface[atom_pos];

    count += addUniqueBridgeAtom(r, nbor_id, atom);
    
  } while (count < no_bridge_atoms);

  bridge_rules->push_back(r);
}



void
ContextGenerator::generate_bridge_rule_list(std::size_t id)
{
  do
    {
      bridge_rules->clear();

      for (std::size_t i = 0; i < no_bridge_rules; ++i)
	{
	  if (rand() % 2)
	    {
	      generate_bridge_rule(id);
	    }
	}
    }
  while (!cover_neighbors(id));
}



bool
ContextGenerator::cover_neighbors(std::size_t id)
{
  std::set<std::size_t> nbors;
  
  BridgeRules::const_iterator i = bridge_rules->begin(); 
  for (; i != bridge_rules->end(); ++i)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*i);
      const NegativeBridgeBody& nb = getNegativeBody(*i);
      
      for (PositiveBridgeBody::const_iterator j = pb.begin(); j != pb.end(); ++j)
	{
	  nbors.insert(j->first);
	  //std::cerr << "insert neighbor id" << j->first << std::endl;
	}
      
      for (NegativeBridgeBody::const_iterator j = nb.begin(); j != nb.end(); ++j)
	{
	  nbors.insert(j->first);
	  //std::cerr << "insert neighbor id" << j->first << std::endl;
	}
    }
  
  // this only works if we generate bridge rules from neighbors.
  // otw, we have to compare the actual neighbors, not just the number of them

  // std::cerr << nbors.size() << std::endl;
  //std::cerr << (*orig_topo)[i-1]->size()) << std::endl;

  return (nbors.size() == (*orig_topo)[id-1]->size());
}



void
ContextGenerator::write_local_kb(std::size_t id)
{
  std::stringstream out;
  out << id;

  std::string filename_lp = prefix + "-" + out.str() + LP_EXT;

  std::ofstream file_lp;
  file_lp.open(filename_lp.c_str());
 
  std::ostringstream oss;
  LogicVisitor lv(oss);
  for (Rules::const_iterator it = local_kb->begin(); it != local_kb->end(); ++it)
    {
      lv.visitRule(*it, id);
    }
  file_lp << oss.str();
  file_lp.close();
}



void
ContextGenerator::write_bridge_rules(std::size_t id)
{
  std::stringstream out;
  out << id;

  std::string filename_br = prefix + "-" + out.str() + BR_EXT;

  std::ofstream file_br;
  file_br.open(filename_br.c_str());
 
  std::ostringstream oss;
  LogicVisitor lv(oss);
  for (BridgeRules::const_iterator it = bridge_rules->begin(); it != bridge_rules->end(); ++it)
    {
      lv.visitBridgeRule(*it, id);
    }
  file_br << oss.str();
  file_br.close();  
}

} // namespace generator
} // namespace dmcs 
