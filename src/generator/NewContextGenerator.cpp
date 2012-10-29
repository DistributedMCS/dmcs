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
 * @file   NewContextGenerator.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sat Oct  27 22:44:20 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "generator/NewLogicVisitor.h"
#include "generator/NewContextGenerator.h"
#include "mcs/BeliefStateOffset.h"

#include "dmcs/Log.h"

#include <iostream>
#include <sstream>
#include <fstream>


namespace dmcs { namespace generator {

int
NewContextGenerator::sign()
{
  return ( rand() % 2 )  ?  1  :  -1;
}



void
NewContextGenerator::generate()
{
  std::size_t system_size = context_interfaces->size();

  for (std::size_t i = 0; i < system_size; ++i)
    {
      local_kb->clear();
      bridge_rules->clear();

      generate_local_kb();
      
      NeighborVecPtr neighbors = (*orig_topo)[i];
      if (neighbors->size() > 0)
	{
	  generate_bridge_rule_list(i);
	}

      write_local_kb(i);
      write_bridge_rules(i);



      update_min_V();

      // Compute local interface between context i and all of its
      // neighbors. Store this information in a map from pairs <i,j>
      // to a BeliefSet. We need this information later for creating
      // the optimal interface wrt. the optimal topology
      
      const NeighborVecPtr nbors = (*orig_topo)[i];
      for (NeighborVec::const_iterator jt = nbors->begin(); jt != nbors->end(); ++jt)
	{
	  // get local interface from context[i] to context[*jt]
	  NewBeliefStatePtr V = local_interface(*jt);
	  
	  ContextPair e(i, *jt);
	  lcim->insert(LocalInterfacePair(e, V));
	}
    }

#if 0
  // Finally, set interface of the root context to all 1, except for
  // the case of ring.
  if (topology_type != PURE_RING_TOPOLOGY && topology_type != RING_EDGE_TOPOLOGY
      && topology_type != MULTIPLE_RING_TOPOLOGY)
    {
      SignaturePtr& bs = (*sigmas)[0];
      std::size_t range = bs->size();
      (*minV)[0] = maxBeliefSet(range);
    }

  DMCS_LOG_TRACE("DONE");
#endif
}



void
NewContextGenerator::generate_local_kb()
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

  DMCS_LOG_TRACE("Adding atom " << neighbor_atom << " of neighbor " << neighbor_id);

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
NewContextGenerator::generate_bridge_rule(std::size_t id)
{
  HeadPtr head(new Head);
  PositiveBridgeBodyPtr pbody(new PositiveBridgeBody);
  NegativeBridgeBodyPtr nbody(new NegativeBridgeBody);
  std::pair<PositiveBridgeBodyPtr, NegativeBridgeBodyPtr> body(pbody, nbody);
  BridgeRulePtr r(new BridgeRule(head, body));

  std::size_t br_h = (rand() % no_atoms) + 1;
  head->push_back(br_h);

  // either 1 or 2 bridge atom(s)
  std::size_t no_bridge_atoms = (rand() % 2) + 1; 

  // loop until enough unique atoms
  std::size_t count = 0;

  NeighborVecPtr nbors = (*orig_topo)[id];

  do {
    std::size_t nbors_size = nbors->size();

    std::size_t nbor_pos  = (rand() % nbors_size);
    std::size_t nbor_id   = (*nbors)[nbor_pos];

    Interface nbor_interface = (*context_interfaces)[nbor_id];
    
    std::size_t no_interface_atoms = nbor_interface.size();
    std::size_t atom_pos = rand() % no_interface_atoms;
    int atom = sign() * nbor_interface[atom_pos];

    count += addUniqueBridgeAtom(r, nbor_id, atom);
    
  } while (count < no_bridge_atoms);

  bridge_rules->push_back(r);
}



void
NewContextGenerator::generate_bridge_rule_list(std::size_t id)
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
NewContextGenerator::cover_neighbors(std::size_t id)
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
	  DMCS_LOG_TRACE("insert neighbor id" << j->first);
	}
      
      for (NegativeBridgeBody::const_iterator j = nb.begin(); j != nb.end(); ++j)
	{
	  nbors.insert(j->first);
	  DMCS_LOG_TRACE("insert neighbor id" << j->first);
	}
    }
  
  // this only works if we generate bridge rules from neighbors.
  // otw, we have to compare the actual neighbors, not just the number of them

  DMCS_LOG_TRACE("neighbor size: " << nbors.size());
  DMCS_LOG_TRACE("orig nb " << id << ", size: " << ( (*orig_topo)[id-1]->size() ) );

  return (nbors.size() == (*orig_topo)[id]->size());
}



void
NewContextGenerator::write_local_kb(std::size_t id)
{
  std::stringstream out;
  out << id;

  std::string filename_lp = prefix + "-" + out.str() + LP_EXT;

  std::ofstream file_lp;
  file_lp.open(filename_lp.c_str());
 
  std::ostringstream oss;
  NewLogicVisitor lv(oss);
  for (Rules::const_iterator it = local_kb->begin(); it != local_kb->end(); ++it)
    {
      lv.visitRule(*it, id, (*sigma_vec)[id]);
    }
  file_lp << oss.str();
  file_lp.close();
}



void
NewContextGenerator::write_bridge_rules(std::size_t id)
{
  std::stringstream out;
  out << id;

  std::string filename_br = prefix + "-" + out.str() + BR_EXT;

  std::ofstream file_br;
  file_br.open(filename_br.c_str());
 
  std::ostringstream oss;
  NewLogicVisitor lv(oss);

  for (BridgeRules::const_iterator it = bridge_rules->begin(); it != bridge_rules->end(); ++it)
    {
      lv.visitBridgeRule(*it, id, sigma_vec);
    }

  file_br << oss.str();
  file_br.close();  
}



NewBeliefStatePtr
NewContextGenerator::local_interface(/* std::size_t id1, */ std::size_t id2)
{
  // with the assumption that bridge_rules now contains bridge rules
  // of context id1 and id2 is one of its neighbors.
  // id1 is therefore IMPLICIT in this context.

  NewBeliefStatePtr lc(new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
					  BeliefStateOffset::instance()->SIZE_BS()));

  lc->setEpsilon(id2, BeliefStateOffset::instance()->getStartingOffsets());

  const BeliefTablePtr sig = (*sigma_vec)[id2];

  for (BridgeRules::const_iterator r = bridge_rules->begin(); r != bridge_rules->end(); ++r)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*r);
      const NegativeBridgeBody& nb = getNegativeBody(*r);
      
      for (PositiveBridgeBody::const_iterator i = pb.begin(); i != pb.end(); ++i)
	if (i->first == id2)
	  lc->set(i->first, i->second, BeliefStateOffset::instance()->getStartingOffsets());
      
      for (NegativeBridgeBody::const_iterator i = nb.begin(); i != nb.end(); ++i)
	if (i->first == id2)
	  lc->set(i->first, i->second, BeliefStateOffset::instance()->getStartingOffsets());
    }

  return lc;
}



void
NewContextGenerator::update_min_V()
{
  std::size_t context_id;
  std::size_t atom_id;

  // update minV based on the bridge rules just been generated
  for (BridgeRules::const_iterator it = bridge_rules->begin(); it != bridge_rules->end(); ++it)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*it);
      const NegativeBridgeBody& nb = getNegativeBody(*it);
      
      for (PositiveBridgeBody::const_iterator k = pb.begin(); k != pb.end(); ++k)
	{
	  context_id = k->first;
	  atom_id = k->second;
	  minV->set(context_id, atom_id, BeliefStateOffset::instance()->getStartingOffsets());
	}
      
      for (NegativeBridgeBody::const_iterator k = nb.begin(); k != nb.end(); ++k)
	{
	  context_id = k->first;
	  atom_id = k->second;
	  minV->set(context_id, atom_id, BeliefStateOffset::instance()->getStartingOffsets());
	}
    }
}

} // namespace generator
} // namespace dmcs 

// Local Variables:
// mode: C++
// End:
