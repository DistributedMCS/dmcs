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
 * @file   BridgeRulesBuilder.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  30 15:29:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef BRIDGE_RULES_BUILDER_TCC
#define BRIDGE_RULES_BUILDER_TCC

#include "SpiritDebugging.h"
#include <boost/functional/hash.hpp>
#include <iostream>
#include <algorithm>

namespace dmcs {

template<typename Grammar>
BridgeRulesBuilder<Grammar>::BridgeRulesBuilder(std::size_t ctx_id_, 
						BridgeRulesPtr& bridge_rules_, 
						NeighborListPtr& neighbor_list_,
						SignatureVecPtr& global_sigs_)
  : ctx_id(ctx_id_),
    bridge_rules(bridge_rules_),
    neighbor_list(neighbor_list_),
    global_sigs(global_sigs_)
{ }

template<typename Grammar>
void
BridgeRulesBuilder<Grammar>::buildNode(typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::BridgeRule);
  HeadPtr head(new Head);
  PositiveBridgeBodyPtr pbody(new PositiveBridgeBody);
  NegativeBridgeBodyPtr nbody(new NegativeBridgeBody);
  std::pair<PositiveBridgeBodyPtr, NegativeBridgeBodyPtr> body(pbody, nbody);
  BridgeRulePtr r(new BridgeRule(head, body));

  build_disjunctive_head(node.children[0], r);
  if (node.children.size() > 1)
    {
      build_body(node.children[1], r);
    }

  bridge_rules->push_back(r);
  neighbor_list->sort();
  neighbor_list->unique();
}

template<typename Grammar>
void
BridgeRulesBuilder<Grammar>::build_disjunctive_head(typename BaseBuilder<Grammar>::node_t& node, 
						    BridgeRulePtr& r)
{
  assert(node.value.id() == Grammar::Disj);

  for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
      it != node.children.end(); ++it)
    {

      std::string atom_name = BaseBuilder<Grammar>::createStringFromNode(it->children[0]);

      SignaturePtr& local_sig = (*global_sigs)[ctx_id - 1];
      const SignatureBySym& local_sig_sym = boost::get<Tag::Sym>(*local_sig);
      SignatureBySym::const_iterator loc_it = local_sig_sym.find(atom_name);      
      // getHead(r)->push_back(loc_it->localId);
      r->first->push_back(loc_it->localId);
    }
}

template<typename Grammar>
void
BridgeRulesBuilder<Grammar>::build_body(typename BaseBuilder<Grammar>::node_t& node, BridgeRulePtr& r)
{
  assert(node.value.id() == Grammar::BridgeBody);

  for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
      it != node.children.end(); ++it)
    {
      if (it->children.size() == 1)
	// positive atom
	{
	  BridgeAtom bap = build_bridge_atom(it->children[0]);
	  // getPositiveBody(r)->push_back(bap);
	  r->second.first->push_back(bap);
	}
      else
	// negative atom
	{
	  BridgeAtom bap = build_bridge_atom(it->children[1]);
	  // getNegativeBody(r)->push_back(bap);
	  r->second.second->push_back(bap);
	}
    }
}

template<typename Grammar>
BridgeAtom
BridgeRulesBuilder<Grammar>::build_bridge_atom(typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::BridgeAtom);
  std::string context_id = BaseBuilder<Grammar>::createStringFromNode(node.children[0]);
  std::string atom_name = BaseBuilder<Grammar>::createStringFromNode(node.children[1]);

  SignaturePtr& local_sig = (*global_sigs)[ctx_id - 1];
  SignatureBySym& local_sig_sym = boost::get<Tag::Sym>(*local_sig);
  SignatureBySym::iterator loc_it = local_sig_sym.find(atom_name);   

  // did not find local id for atom_name, generate one
  if(loc_it == local_sig_sym.end())
    {
      std::size_t neighbor_id = std::atoi(context_id.c_str());

      const std::string empty_str = "";

      // this is one of my neighbor
      NeighborList::const_iterator it = std::find_if(neighbor_list->begin(), neighbor_list->end(), compareNeighbors(neighbor_id));

      if (it == neighbor_list->end())
	{
	  NeighborPtr n(new Neighbor(neighbor_id, "", ""));
	  neighbor_list->push_back(n);
	}

      const SignaturePtr neighborSignature = (*global_sigs)[neighbor_id - 1];
    
      const SignatureBySym& neighbor_sig_sym = boost::get<Tag::Sym>(*neighborSignature);
      SignatureBySym::const_iterator neighbor_it = neighbor_sig_sym.find(atom_name);

      assert(neighbor_it != neighbor_sig_sym.end());

      std::size_t local_id_here = local_sig->size() + 1;
      local_sig->insert(Symbol(neighbor_it->sym, neighbor_it->ctxId, local_id_here, neighbor_it->origId));
    }

  // if local_sig automatically reflects the change of sig, then this
  //should be ok, otherwise we have to get the signature by id again
  //from the sig.  might need to remove this

  local_sig_sym = boost::get<Tag::Sym>(*local_sig);
  loc_it = local_sig_sym.find(atom_name);      
  assert(loc_it !=local_sig_sym.end() );
  BridgeAtom ba(std::atoi(context_id.c_str()), loc_it->localId);
  return ba;
}

} // namespace dmcs

#endif // BRIDGE_RULES_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
