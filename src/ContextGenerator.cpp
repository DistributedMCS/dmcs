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
      
      NeighborVecPtr neighbors = (*orig_topo)[i-1];
      if (neighbors->size() > 0)
	{
	  generate_bridge_rule_list(i);
	}

      write_local_kb(i);
      write_bridge_rules(i);

      // Update min V
      update_min_V();

      // Compute local interface between context i and all of its
      // neighbors. Store this information in a map from pairs <i,j>
      // to a BeliefSet. We need this information later for creating
      // the optimal interface wrt. the optimal topology
      
      const NeighborVecPtr nbors = (*orig_topo)[i-1];
      for (NeighborVec::const_iterator jt = nbors->begin(); jt != nbors->end(); ++jt)
	{
	  BeliefSet lci = local_interface(i, *jt);
	  BeliefStatePtr V(new BeliefState(system_size, 0));
	  (*V)[*jt - 1] = lci;
	  
	  ContextPair e(i, *jt);
	  lcim->insert(LocalInterfacePair(e, V));
	}
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

  NeighborVecPtr nbors = (*orig_topo)[id-1];

  do {
    std::size_t nbors_size = nbors->size();

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



BeliefSet
ContextGenerator::local_interface(std::size_t id1, std::size_t id2)
{
  // with the assumption that bridge_rules now contains bridge rules
  // of context id1 and id2 is one of its neighbors

  BeliefSet lc = setEpsilon(0);

  const SignaturePtr sig = (*sigmas)[id2-1];

  for (BridgeRules::const_iterator r = bridge_rules->begin(); r != bridge_rules->end(); ++r)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*r);
      const NegativeBridgeBody& nb = getNegativeBody(*r);
      
      for (PositiveBridgeBody::const_iterator i = pb.begin(); i != pb.end(); ++i)
	{
	  if (i->first == id2)
	    {
	      const SignatureByLocal& local = boost::get<Tag::Local>(*sig);
	      SignatureByLocal::const_iterator loc_it = local.find(i->second);
	      lc = setBeliefSet(lc, loc_it->localId);
	    }
	}
      
      for (NegativeBridgeBody::const_iterator i = nb.begin(); i != nb.end(); ++i)
	{
	  if (i->first == id2)
	    {
	      const SignatureByLocal& local = boost::get<Tag::Local>(*sig);
	      SignatureByLocal::const_iterator loc_it = local.find(i->second);
	      lc = setBeliefSet(lc, loc_it->localId);
	    }
	}
    }

  return lc;
}



void
ContextGenerator::update_min_V()
{
  std::size_t context_id;
  std::size_t atom_id;

  // update minV based on the bridge rules just been generated
  BeliefState& v_state = *minV;

  for (BridgeRules::const_iterator it = bridge_rules->begin(); it != bridge_rules->end(); ++it)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*it);
      const NegativeBridgeBody& nb = getNegativeBody(*it);
      
      for (PositiveBridgeBody::const_iterator k = pb.begin(); k != pb.end(); ++k)
	{
	  context_id = k->first;
	  atom_id = k->second;
	  
	  BeliefSet& vb = v_state[context_id - 1];
	  
	  // in V: turn on the corresponding bit in the corresponding context
	  vb = setBeliefSet(vb, atom_id);
	}
      
      for (NegativeBridgeBody::const_iterator k = nb.begin(); k != nb.end(); ++k)
	{
	  context_id = k->first;
	  atom_id = k->second;
	  
	  BeliefSet& vb = v_state[context_id - 1];
	  
	  // in V: turn on the corresponding bit in the corresponding context
	  vb = setBeliefSet(vb, atom_id);
	}
    }
}

} // namespace generator
} // namespace dmcs 

// Local Variables:
// mode: C++
// End:
