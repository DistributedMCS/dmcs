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

#include "parser/SpiritDebugging.h"

namespace dmcs {

template<typename Grammar>
ClaspResultBuilder<Grammar>::ClaspResultBuilder(const ProxySignatureByLocal& context_signature,
						const BeliefStateListPtr& belief_states_,
						std::size_t sys_size)
  : BaseBuilder<Grammar>(Grammar()), 
    local_sig(context_signature),
    system_size(sys_size),
    belief_states(belief_states_)
{ }


template<typename Grammar>
void
ClaspResultBuilder<Grammar>::buildNode (typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::Value); // we expect a model

  assert(node.children.size() == local_sig.size()); // a model must have sig_size literals

  //
  // create a new belief state and dispatch content of the model represented in node
  //

  BeliefStatePtr bs(new BeliefState(system_size, 0)); // initially, all is zero
  
  // set all literals in the belief state based on the model in node
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

  ///@todo TK: find a way to get integers from the model
  std::string str_lit = BaseBuilder<Grammar>::createStringFromNode(node.children[0]);

  ///@todo no error checking??
  Literal local_lit = std::atoi(str_lit.c_str());
  
  // we only allow non-zero literals in range of the signature, as 0
  // is used as sentinel
  assert(std::abs(local_lit) > 0 && std::abs(local_lit) <= (int)local_sig.size());

  // find the global value for the belief state
  SignatureByLocal::const_iterator loc_it = local_sig.find(std::abs(local_lit));

  // it must show up in the signature
  assert (loc_it != local_sig.end());

  // now setup belief state at the right position
  std::size_t cid = loc_it->ctxId - 1;

  //  std::cerr << "Have atom: " << std::abs(local_lit) << "in context " << cid+1 << std::endl;
  
  BeliefSet& b = (*bs)[cid];

  // just to be safe
  assert(cid < system_size);
  
  // un/set bit in the right context at the right position
  if (local_lit > 0) // positive literal
    {
      b.set(loc_it->origId, true);
    }
  else // negative literal (this is probably not needed)
    {
      b.set(loc_it->origId, false);
    }
  
  // turn on epsilon of neighbor here, because we don't have
  // enough information about neighbors from the graph (some edges
  // were removed).
  setEpsilon(b);
}

} // namespace dmcs

#endif // _CLASP_RESULT_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
