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

#include "ContextGenerator.h"


using namespace dmcs;
using namespace dmcs::generator;


int
ContextGenerator::sign()
{
  return ( rand() % 2 )  ?  1  :  -1;
}


void
ContextGenerator::generate()
{
  for (Contexts::const_iterator i = contexts->begin(); i != contexts->end(); ++i)
    {
      generate_local_kb(*i);

      int context_id = (*i)->getContextID();
      //std::cerr << "In ContextGenerator::generate" << std::endl;
      //query_plan->write_graph(std::cerr);
      //std::cerr << std::endl;

      NeighborListPtr neighbors = query_plan->getNeighbors1(context_id);
      /*std::cerr << "Our neighbor:" << std::endl;
      for (Neighbors_::const_iterator j = neighbors->begin(); j != neighbors->end(); ++j)
	{
	  std::cerr << *j << " ";
	}
	std::cerr << std::endl;*/
	
      if (neighbors->size() > 0)
	{
	  generate_bridge_rules(*i);
	}
    }
}


std::size_t
addUniqueBridgeAtom(BridgeRulePtr& r, std::size_t neighbor_id, int neighbor_atom)
{
  BridgeAtom bap = std::make_pair(neighbor_id, std::max(neighbor_atom, -neighbor_atom));

  std::cerr << "Adding atom " << neighbor_atom << " of neighbor " << neighbor_id << std::endl;

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


bool
ContextGenerator::cover_neighbors(const ContextPtr& context)
{
  std::set<std::size_t> nbors;
  
  BridgeRulesPtr br = context->getBridgeRules();


  for (BridgeRules::const_iterator i = br->begin(); i != br->end(); ++i)
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
  int context_id = context->getContextID();

  // std::cerr << nbors.size() << std::endl;
  //std::cerr << query_plan->getNeighbors(context_id)->size() << std::endl;

  return (nbors.size() == query_plan->getNeighbors1(context_id)->size());
}

void
ContextGenerator::generate_local_kb(const ContextPtr& context)
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
	      context->getLocalKB()->push_back(r);
	    }
	  else
	    {
	      // going forward, need to check
	      if (i < no_atoms - 1)
		{
		  nbody->push_back(i+1);
		  context->getLocalKB()->push_back(r);
		}
	    }
	}
      else
	{
	  // going forward, need to check
	  if (i < no_atoms - 1)
	    {
	      nbody->push_back(i+1);
	      context->getLocalKB()->push_back(r);
	    }
	}
    }
}


void
ContextGenerator::generate_bridge_rule(const ContextPtr& context)
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
  int i = 0;
  int context_id = context->getContextID();
  NeighborListPtr nbors = query_plan->getNeighbors1(context_id);

  /*
  std::cerr << "Our neighbor:" << std::endl;
  for (Neighbors_::const_iterator i = nbors->begin(); i != nbors->end(); ++i)
    {
      std::cerr << *i << " ";
    }
    std::cerr << std::endl;*/
  

  do
    {

      int neighbors_size = nbors->size();
      int neighbor_pos = (rand() % neighbors_size);
      NeighborList::const_iterator it = nbors->begin();
      std::advance(it, neighbor_pos);
      NeighborPtr nb = *it;
      std::size_t neighbor_id = nb->neighbor_id;//(*nbors)[neighbor_pos];

      //int atom = sign() * ((rand() % no_atoms) + 1);
      std::size_t atom_pos = rand() % no_interface_atoms;

      std::cerr << "Neighbor = " << nb->neighbor_id << std::endl;
      Interface interface_neighbor = context_interfaces[neighbor_id-1];

      for (std::size_t it = 0; it < no_interface_atoms; ++it)
	{
	  std::cerr << interface_neighbor[it] << " ";
	}
      std::cerr << std::endl;

      std::cerr << "atom_pos = " << atom_pos << " --> atom = " << interface_neighbor[atom_pos] << std::endl;

      int atom = sign() * interface_neighbor[atom_pos];

      i += addUniqueBridgeAtom(r, neighbor_id, atom);

      /*
      std::cerr << "i: " << i << std::endl;
      std::cerr << "no bridge atoms: " << no_bridge_atoms << std::endl;
      std::cerr << "neighbor id: " << neighbor_id << std::endl;      
      std::cerr << "neighbor atom: " << atom << std::endl;
      */
    }
  while (i < no_bridge_atoms);
  //std::cerr << "Got out of the loop" << std::endl;

  context->getBridgeRules()->push_back(r);
}

void
ContextGenerator::generate_bridge_rules(const ContextPtr& context)
{
  do
    {
      context->getBridgeRules()->clear();

      for (std::size_t i = 0; i < no_bridge_rules; ++i)
	{
	  if (rand() % 2)
	    {
	      generate_bridge_rule(context);
	    }
	}
    }
  while (!cover_neighbors(context));
}
