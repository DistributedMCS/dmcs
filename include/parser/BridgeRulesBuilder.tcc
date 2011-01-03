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

#include "parser/SpiritDebugging.h"

#include <boost/functional/hash.hpp>
#include <iostream>
#include <algorithm>

#ifdef DEBUG
#include <bitset>
#endif

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
  assert(node.value.id() == Grammar::SchematicBridgeRule);
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

#ifdef DEBUG
  std::cerr << "Got new rule r = " << r << std::endl;
#endif

  vm.clear();
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
  assert(node.value.id() == Grammar::SchematicBridgeBody);

  for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
      it != node.children.end(); ++it)
    {
      if (it->children.size() == 1)
	// positive atom
	{
	  BridgeAtom bap = build_bridge_atom(it->children[0]);

#ifdef DEBUG  
	  std::cerr << "Got positive bridge atom:" << bap << std::endl;
#endif

	  // getPositiveBody(r)->push_back(bap);
	  r->second.first->push_back(bap);
	}
      else
	// negative atom
	{
	  BridgeAtom bap = build_bridge_atom(it->children[1]);

#ifdef DEBUG  
	  std::cerr << "Got negative bridge atom:" << bap << std::endl;
#endif

	  // getNegativeBody(r)->push_back(bap);
	  r->second.second->push_back(bap);
	}
    }
}


template<typename Grammar>
BridgeAtom
BridgeRulesBuilder<Grammar>::build_bridge_atom(typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::SchematicBridgeAtom);
  std::string str_context_term = BaseBuilder<Grammar>::createStringFromNode(node.children[0]);
  
  // 1 -- ordinary belief:           p
  // 2 -- similar schematic belief: [p]
  // 3 -- exact schematic belief:  @[p]
  std::size_t no_of_children = node.children[1].children.size();
  std::size_t sc_position = no_of_children - 1;
  std::size_t schematic_type = 0;

  assert (ctx_id <= global_sigs->size());

  switch (no_of_children)
    {
    case 1:
      schematic_type = IS_ORDINARY_BELIEF;
      break;
    case 2:
      schematic_type = IS_SIMILAR_SCHEMATIC_BELIEF;
      break;
    case 3:
      schematic_type = IS_EXACT_SCHEMATIC_BELIEF;
      break;
    default:
      assert(false);
    }

  typename BaseBuilder<Grammar>::node_t& node_sb = node.children[1].children[sc_position];

  std::string str_schematic_const = BaseBuilder<Grammar>::createStringFromNode(node_sb);

#ifdef DEBUG
  std::cerr << "Facing: " << str_context_term << ", " << str_schematic_const << std::endl;
  std::cerr << "Context Variables counter = " << ctx_var_counter << std::endl;
#endif

  ContextTerm context_term;
  SchematicBelief schematic_belief;

  std::size_t c = str_context_term[0];
  if ((c >= 'A') && (c <= 'Z'))
    // face a context variable
    {
      // check whether this variable already parsed in this rule
      std::map<std::string, ContextTerm>::const_iterator v_it = vm.find(str_context_term);
      if (v_it == vm.end())
	{
	  context_term = ctxVarTerm(ctx_id, ++ctx_var_counter);
	  vm.insert(std::pair<std::string, ContextTerm>(str_context_term, context_term));
	}
      else
	// this context varialbe already appeared IN THIS RULE
	{
	  context_term = v_it->second;
	}

      // in the case of context variable, the schematic constant must
      // be in the signature of the current context
      const SignaturePtr& local_sig = (*global_sigs)[ctx_id - 1];
      const SignatureBySym& local_sig_sym = boost::get<Tag::Sym>(*local_sig);
      SignatureBySym::const_iterator loc_it = local_sig_sym.find(str_schematic_const);

      assert(loc_it != local_sig_sym.end());

      std::size_t schematic_const = loc_it->origId;
      schematic_belief = constructSchematicBelief(schematic_type, schematic_const);

#ifdef DEBUG
      std::bitset<sizeof(ContextTerm)*8> context_term_sb = context_term;
      std::bitset<sizeof(SchematicBelief)*8> schematic_belief_sb = schematic_belief;

      std::cerr << "str_context_term    = " << str_context_term << std::endl 
		<< "encoded             = " << context_term << std::endl
		<< "                    = " << context_term_sb << std::endl;
      std::cerr << "isVar = " << isCtxVar(context_term) << ", ctx_id = " << ctxID(context_term);
      std::cerr << ", var = " << varID(context_term) << std::endl;
      
      std::cerr << "str_schematic_const = " << str_schematic_const << std::endl
		<< "encoded             = " << schematic_belief << std::endl
		<< "                    = " << schematic_belief_sb << std::endl;
      std::cerr << "type =" << sBeliefType(schematic_belief) << ", schematic_const = " << sBelief(schematic_belief) << std::endl;
#endif
    }
  else
    {
      // face a context id
      // for the moment, assume that there is no nasty input
      // concerning the context id
      std::size_t context_id = std::atoi(str_context_term.c_str());
      context_term = ctxConstTerm(context_id);

      NeighborList::const_iterator it = std::find_if(neighbor_list->begin(), neighbor_list->end(), compareNeighbors(context_id));

      if (it == neighbor_list->end())
	{
	  NeighborPtr n(new Neighbor(context_id, "", ""));
	  neighbor_list->push_back(n);
	}


      const SignaturePtr& local_sig = (*global_sigs)[ctx_id - 1];
      SignatureBySym& local_sig_sym = boost::get<Tag::Sym>(*local_sig);
      SignatureBySym::iterator loc_it = local_sig_sym.find(str_schematic_const);

      // did not find local id for the schematic constant, generate one
      if(loc_it == local_sig_sym.end())
	{
	  const SignaturePtr& neighborSignature = (*global_sigs)[context_id-1];
	  
	  const SignatureBySym& neighbor_sig = boost::get<Tag::Sym>(*neighborSignature);
	  SignatureBySym::const_iterator neighbor_it = neighbor_sig.find(str_schematic_const);
	  
	  assert(neighbor_it != neighbor_sig.end());
	  
	  std::size_t local_id_here = local_sig->size() + 1;
	  local_sig->insert(Symbol(neighbor_it->sym, neighbor_it->ctxId, local_id_here, neighbor_it->origId));
	}

      // if local_sig automatically reflects the change of sig, then
      // this should be ok, otherwise we have to get the signature by
      // id again from the sig. might need to remove this
      
      local_sig_sym = boost::get<Tag::Sym>(*local_sig);
      loc_it = local_sig_sym.find(str_schematic_const);      
      assert(loc_it != local_sig_sym.end() );

      schematic_belief = loc_it->localId;

#ifdef DEBUG
      std::bitset<sizeof(ContextTerm)*8> context_term_sb = context_term;
      std::bitset<sizeof(SchematicBelief)*8> schematic_belief_sb = schematic_belief;

      std::cerr << "str_context_term    = " << str_context_term << std::endl 
		<< "encoded             = " << context_term << std::endl
		<< "                    = " << context_term_sb << std::endl;
      std::cerr << "isVar = " << isCtxVar(context_term) << std::endl;

      std::cerr << "str_schematic_const = " << str_schematic_const << std::endl
		<< "encoded             = " << schematic_belief << std::endl
		<< "                    = " << schematic_belief_sb << std::endl;
      std::cerr << "type =" << sBeliefType(schematic_belief) << ", schematic_const = " << sBelief(schematic_belief) << std::endl;
#endif
    }

#ifdef DEBUG
  std::cerr << "Create atom (" << context_term << ":" << schematic_belief << ")" << std::endl;
#endif  

  BridgeAtom ba(context_term, schematic_belief);

  return ba;
}

} // namespace dmcs

#endif // BRIDGE_RULES_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
