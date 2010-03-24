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
 * @file   ClaspResultBuilder.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:40:08 2009
 * 
 * @brief  
 * 
 * 
 */


#if !defined(_CLASP_RESULT_BUILDER_TCC)
#define _CLASP_RESULT_BUILDER_TCC

#include "SpiritDebugging.h"

namespace dmcs {

template<typename Grammar>
ClaspResultBuilder<Grammar>::ClaspResultBuilder(const Context& context_,
						BeliefStatesPtr& belief_states_)
  : context(context_),
    belief_states(belief_states_)
{ }


template<typename Grammar>
void
ClaspResultBuilder<Grammar>::buildNode (typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::ClaspAnswer);

  BeliefStatePtr bs(new BeliefState(context.getSystemSize())); // initially, all is zero

  ///@todo this may be optimized

  // mark this belief set as non-epsilon
  bs.belief_state_ptr->belief_state[context.getContextID() - 1] = 1;

  NeighborsPtr_ neighbors= context.getQueryPlan()->getNeighbors(context.getContextID());
  // activate neighbors as non-epsilons
  
  /*for (Neighbors_::const_iterator it = neighbors->begin(); 
       it != neighbors->end(); ++it)
    {
      std::cerr << "Turn on neighbor " << *it << std::endl;
      bs.belief_state_ptr->belief_state[(*it) - 1] = 1;
      }*/
  
  
  // set all positive literals in the belief state
  for (typename BaseBuilder<Grammar>::node_t::tree_iterator jt = node.children.begin(); 
       jt != node.children.end(); ++jt)
    {
      add_atom(*jt, bs);
    }

  belief_states.belief_states_ptr->belief_states.insert(bs);
}


template<typename Grammar>
void
ClaspResultBuilder<Grammar>::add_atom(typename BaseBuilder<Grammar>::node_t& node, BeliefStatePtr& bs)
{
  BeliefSets& bstate = bs.belief_state_ptr->belief_state;
  if (node.children.size() == 1) // only positive atoms
    {
      ///@todo can be optimized
      // turn on the respective bit in bs
      std::string str_lit = BaseBuilder<Grammar>::createStringFromNode(node.children[0]);
      Literal local_lit = std::atoi(str_lit.c_str());

      // we only allow positive literals
      assert(local_lit > 0);
      
      // look-up signature for the position in the belief state
      const SignatureByLocal& local_sig = boost::get<Tag::Local>(*context.getSignature());
      SignatureByLocal::const_iterator loc_it = local_sig.find(local_lit);
      

      // it must show up in the signature
      if(loc_it != local_sig.end())
	{
	  // set bit in the right context at the right position
	  bstate[loc_it->ctxId - 1] |= (0x1 << loc_it->origId);

	  // turn on neighbor here, because we don't have enough information 
	  // about neighbors from the graph (some edges were removed).
	  bstate[loc_it->ctxId - 1] |= 1;
	}
    }
  else
    {
      std::string str_lit = BaseBuilder<Grammar>::createStringFromNode(node.children[1]);
      Literal local_lit = std::atoi(str_lit.c_str());

      const SignatureByLocal& local_sig = boost::get<Tag::Local>(*context.getSignature());
      SignatureByLocal::const_iterator loc_it = local_sig.find(local_lit);
      

      // it must show up in the signature
      if(loc_it != local_sig.end())
	{
	  // turn on neighbor here, because we don't have enough information 
	  // about neighbors from the graph (some edges were removed).
	  // However, we don't need to set bit because this is a false atom.
	  bstate[loc_it->ctxId - 1] |= 1;
	}
    }
}

} // namespace dmcs

#endif // _CLASP_RESULT_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
