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
						BeliefStateListPtr& belief_states_)
  ///@todo future versions of clasp will have a sentinel
  : BaseBuilder<Grammar>(Grammar(context_.getSignature()->size())), 
    sig(context_.getSignature()),
    local_sig(boost::get<Tag::Local>(*sig)),
    sig_size(sig->size()),
    system_size(context_.getSystemSize()),
    belief_states(belief_states_)
{ }


template<typename Grammar>
void
ClaspResultBuilder<Grammar>::buildNode (typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::Value); // we expect a model

  BeliefStatePtr bs(new BeliefState(system_size, 0)); // initially, all is zero

  ///@todo this may be optimized, I think we can remove the non-epsilon part here...

  // mark this belief set as non-epsilon
  //bs.belief_state_ptr->belief_state[context.getContextID() - 1] = 1;

  //NeighborsPtr_ neighbors= context.getQueryPlan()->getNeighbors(context.getContextID());
  // activate neighbors as non-epsilons
  
  /*for (Neighbors_::const_iterator it = neighbors->begin(); 
       it != neighbors->end(); ++it)
    {
      std::cerr << "Turn on neighbor " << *it << std::endl;
      bs.belief_state_ptr->belief_state[(*it) - 1] = 1;
      }*/
  
  assert(node.children.size() == sig_size); // a model must have sig_size literals
  
  // set all positive literals in the belief state
  for (typename BaseBuilder<Grammar>::node_t::tree_iterator jt = node.children.begin(); 
       jt != node.children.end(); ++jt)
    {
      add_literal(*jt, bs);
    }

  belief_states->push_back(bs); ///@todo sort???
}


template<typename Grammar>
void
ClaspResultBuilder<Grammar>::add_literal(typename BaseBuilder<Grammar>::node_t& node,
					 BeliefStatePtr& bs)
{
  assert(node.children.size() == 1); // we expect that literals have size() == 1

  std::string str_lit = BaseBuilder<Grammar>::createStringFromNode(node.children[0]);

  ///@todo no error checking??
  Literal local_lit = std::atoi(str_lit.c_str());
  
  // we only allow non-zero literals in range of the signature, as 0
  // is used as sentinel
  assert(std::abs(local_lit) > 0 && std::abs(local_lit) <= (int)sig_size);

  // find the global value for the belief state
  SignatureByLocal::const_iterator loc_it = local_sig.find(std::abs(local_lit));

  // it must show up in the signature
  if(loc_it != local_sig.end())
    {
      // now setup belief state at the right position
      BeliefState& bstate = *bs;

      std::size_t cid = loc_it->ctxId - 1;

      // just to be save
      assert(cid < system_size);

      // un/set bit in the right context at the right position
      if (local_lit > 0) // positive literal
	{
	  bstate[cid] |= (0x1 << loc_it->origId);
	}
      else // negative literal (this is probably not needed)
	{
	  bstate[cid] &= ~(0x1 << loc_it->origId);
	}
      
      // turn on epsilon of neighbor here, because we don't have
      // enough information about neighbors from the graph (some edges
      // were removed).
      bstate[cid] |= 1;
    }
}

} // namespace dmcs

#endif // _CLASP_RESULT_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
