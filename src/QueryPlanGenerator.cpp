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
 * @file   QueryPlanGenerator.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  21 17:27:00 2010
 * 
 * @brief  
 * 
 * 
 */

#include "QueryPlanGenerator.h"
#include "Rule.h"
#include "Signature.h"

using namespace dmcs;
using namespace dmcs::generator;


QueryPlanGenerator::QueryPlanGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
  : contexts(contexts_), 
    query_plan(query_plan_), 
    V(new BeliefState(contexts->size()))
{ 
  std::srand ( std::time(NULL) );
}



void
QueryPlanGenerator::generate()
{
  for (Contexts::const_iterator i = contexts->begin(); i != contexts->end(); ++i)
    {
      genNeighbors(*i);
    }
}


void
QueryPlanGenerator::genNeighbor(const ContextPtr& context, std::size_t id)
{
  std::size_t my_id = context->getContextID();
  query_plan->add_neighbor(my_id, id);
}


void 
QueryPlanGenerator::create_interfaces()
{
  // a bit redundant to put local interface initialization here.
  // but when we really implement the optimze algorithm, it doesn't matter.
  initialize_local_interface();
  compute_min_V();
  query_plan->putGlobalV(V);

  Contexts::const_iterator i = contexts->end();
  --i;
  for (; i != contexts->begin(); --i)
    {
      create_interface(*i);
    }
  create_interface(*i);
}


int
QueryPlanGenerator::local_interface(std::size_t context_id1, std::size_t context_id2)
{
  int li = 1;
  Contexts::const_iterator c1 = contexts->begin();
  Contexts::const_iterator c2 = contexts->begin();
  
  std::advance(c1, context_id1 - 1);
  std::advance(c2, context_id2 - 1);
  
  const BridgeRulesPtr& br = (*c1)->getBridgeRules();
  const SignaturePtr& sig = (*c2)->getSignature();
  
  for (BridgeRules::const_iterator r = br->begin(); r != br->end(); ++r)
    {
      const PositiveBridgeBody& pb = getPositiveBody(*r);
      const NegativeBridgeBody& nb = getNegativeBody(*r);
      
      for (PositiveBridgeBody::const_iterator i = pb.begin(); i != pb.end(); ++i)
	{
	  if (i->first == context_id2)
	    {
	      const SignatureByLocal& local = boost::get<Tag::Local>(*sig);
	      SignatureByLocal::const_iterator loc_it = local.find(i->second);
	      li = li | 1 << (loc_it->localId);
	    }
	}
      
      for (NegativeBridgeBody::const_iterator i = nb.begin(); i != nb.end(); ++i)
	{
	  if (i->first == context_id2)
	    {
	      const SignatureByLocal& local = boost::get<Tag::Local>(*sig);
	      SignatureByLocal::const_iterator loc_it = local.find(i->second);
	      li = li | 1 << (loc_it->localId);
	    }
	}
    }
  
  return li;
}


void
QueryPlanGenerator::initialize_local_interface()
{
  EdgesPtr edges = query_plan->getEdges();

  for (Edges::const_iterator e = edges->begin(); e != edges->end(); ++e)
    {
      BeliefStatePtr V(new BeliefState(contexts->size(), 0)); // create empty belief state
      
      (*V)[e->second-1] = local_interface(e->first, e->second);

      //  std::cerr << "initialize local interface " << e->first << " " << e->second << std::endl;
      query_plan->putInterface(e->first, e->second, V);
    }
}


void
QueryPlanGenerator::compute_min_V()
{
  std::size_t context_id;
  std::size_t atom_id;

  Contexts::const_iterator i = contexts->begin();

  BeliefState& v_state = *V;

  for (;i != contexts->end(); ++i)
    {
      v_state[(*i)->getContextID() - 1] = 1;
    }

  for (Contexts::const_iterator i = contexts->begin(); i != contexts->end(); ++i)
    {
      BridgeRulesPtr br = (*i)->getBridgeRules();
      
      for (BridgeRules::const_iterator j = br->begin(); j != br->end(); ++j)
	{
	  const PositiveBridgeBody& pb = getPositiveBody(*j);
	  const NegativeBridgeBody& nb = getNegativeBody(*j);

	  for (PositiveBridgeBody::const_iterator k = pb.begin(); k != pb.end(); ++k)
	    {
	      context_id = k->first;
	      atom_id = k->second;

	      // in V: turn on the corresponding bit in the corresponding context
	      v_state[context_id - 1] |= (1 << atom_id);
	    }

	  for (NegativeBridgeBody::const_iterator k = nb.begin(); k != nb.end(); ++k)
	    {
	      context_id = k->first;
	      atom_id = k->second;

	      // in V: turn on the corresponding bit in the corresponding context
	      v_state[context_id - 1] |= (1 << atom_id);
	    }
	}
    }

  i = contexts->begin();
  context_id = (*i)->getContextID();

  if (v_state[context_id - 1] == 1)
    {
      v_state[context_id - 1] = std::numeric_limits<unsigned long>::max();  
    }
}


const BeliefStatePtr&
QueryPlanGenerator::getMinV() const
{
  return V;
}
