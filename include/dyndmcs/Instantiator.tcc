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
 * @file   Instantiator.tcc
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Oct  05 11:53:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATOR_TCC
#define INSTANTIATOR_TCC

#include "dmcs/Log.h"

namespace dmcs {

template<typename BridgeBodyType>
inline void
instantiate_body(const BridgeBodyType& body, 
		 typename boost::shared_ptr<BridgeBodyType> instantiated_body,
		 SignatureVecPtr global_sigs,
		 NeighborListPtr context_info,
		 NeighborListPtr neighbors,
		 ContextSubstitutionPtr ctx_sub,
		 std::size_t ctx_id)
{
  for (typename BridgeBodyType::const_iterator it = body.begin(); it != body.end(); ++it)
    {
      std::size_t neighbor_id;
      
      ContextTerm ctt = it->first;
      SchematicBelief sb = it->second;
      std::size_t sb_type = sBeliefType(sb);
      
      if (!isCtxVar(ctt) && (sb_type == IS_ORDINARY_BELIEF)) // ordinary s-bridge atom, just copy it over
	{
	  instantiated_body->push_back(*it);
	  neighbor_id = ctt;
	}
      else
	{
	  // find the match for this sbridge atom in ContextSubstitution
	  const ContextSubstitutionBySrcSym& ctt_ss = boost::get<Tag::SrcSym>(*ctx_sub);
	  ContextSubstitutionBySrcSym::const_iterator sub_it = ctt_ss.find(boost::make_tuple(ctt, sBelief(sb)));
	  
	  DMCS_LOG_DEBUG("BridgeAtom = " << *it);
	  DMCS_LOG_DEBUG("ctt = " << ctt);
	  DMCS_LOG_DEBUG("sb  = " << sb);

	  assert ( sub_it != ctt_ss.end() );
	  
	  // now instantiate with the target context and the image atom in that context
	  std::size_t target_context = sub_it->tarCtx;
	  std::size_t target_atom = sub_it->img;
	  
	  neighbor_id = target_context;
	  
	  ContextTerm i_ctt = ctxConstTerm(target_context);
	  SchematicBelief i_sb = constructSchematicBelief(IS_ORDINARY_BELIEF, target_atom);
	  
	  BridgeAtom ba(i_ctt, i_sb);
	  instantiated_body->push_back(ba);
	  
	  // now update the global signature
	  SignaturePtr local_sig = (*global_sigs)[ctx_id-1];
	  SignaturePtr neighbor_sig = (*global_sigs)[target_context-1];
	  
	  // get information of the target atom
	  const SignatureByLocal& neighbor_sig_local = boost::get<Tag::Local>(*neighbor_sig);
	  SignatureByLocal::const_iterator nsl_it = neighbor_sig_local.find(target_atom);
	  
	  assert(nsl_it != neighbor_sig_local.end());
	  
	  std::size_t local_id_here = local_sig->size() + 1;
	  local_sig->insert(Symbol(nsl_it->sym, target_context, local_id_here, target_atom));
	}
      
      // add neighbor_id to the list of neighbors to invoke later
      if (neighbor_id != ctx_id)
	{
	  NeighborList::const_iterator nb = std::find_if(neighbors->begin(), neighbors->end(), compareNeighbors(neighbor_id));
	  if (nb == neighbors->end())
	    {
	      NeighborList::const_iterator nb_info = std::find_if(context_info->begin(), context_info->end(), compareNeighbors(neighbor_id));
	      
	      // information for this neighbor (including hostname
	      // and port) must be available in context_info
	      assert ( nb_info != context_info->end() );

	      DMCS_LOG_DEBUG("Adding neighbor " << neighbor_id);

	      neighbors->push_back(*nb_info);
	    }
	}
    }
}

} // namespace dmcs

#endif // INSTANTIATOR_TCC

// Local Variables:
// mode: C++
// End:
