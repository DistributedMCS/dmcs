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
 * @file   LocalKBBuilder.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  30 15:29:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef LOCAL_KB_BUILDER_TCC
#define LOCAL_KB_BUILDER_TCC

#include "SpiritDebugging.h"
#include <boost/functional/hash.hpp>

namespace dmcs {

template<typename Grammar>
LocalKBBuilder<Grammar>::LocalKBBuilder(RulesPtr& local_kb_, SignaturePtr& sig_)
  : local_kb(local_kb_),
    sig(sig_)
{ }


template<typename Grammar>
void
LocalKBBuilder<Grammar>::buildNode(typename BaseBuilder<Grammar>::node_t& node)
{
  assert(node.value.id() == Grammar::Rule);

  HeadPtr head(new Head);
  PositiveBodyPtr pbody(new PositiveBody);
  NegativeBodyPtr nbody(new NegativeBody);
  std::pair<PositiveBodyPtr, NegativeBodyPtr> body(pbody, nbody);
  RulePtr r(new Rule(head, body));

  build_disjunctive_head(node.children[0], r);
  
  if (node.children.size() > 1)
    {
      build_body(node.children[1], r);
    }

  local_kb->push_back(r);
}

template<typename Grammar>
void
LocalKBBuilder<Grammar>::build_disjunctive_head(typename BaseBuilder<Grammar>::node_t& node, RulePtr& r)
{
  assert(node.value.id() == Grammar::Disj);
 
  for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
      it != node.children.end(); ++it)
    {
      std::string atom_name = BaseBuilder<Grammar>::createStringFromNode(it->children[0]);
      
      const SignatureBySym& local_sig = boost::get<Tag::Sym>(*sig);
      SignatureBySym::const_iterator loc_it = local_sig.find(atom_name);      

      r->first->push_back(loc_it->localId);
    }
}

template<typename Grammar>
void
LocalKBBuilder<Grammar>::build_body(typename BaseBuilder<Grammar>::node_t& node, RulePtr& r)
{
  assert(node.value.id() == Grammar::Body);

  for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
      it != node.children.end(); ++it)
    {
      if (it->children.size() == 1)
	// positive atom
	{
	  std::string atom_name = BaseBuilder<Grammar>::createStringFromNode(it->children[0]);

	  const SignatureBySym& local_sig = boost::get<Tag::Sym>(*sig);
	  SignatureBySym::const_iterator loc_it = local_sig.find(atom_name);      

	  r->second.first->push_back(loc_it->localId);
	}
      else
	// negative atom
	{
	  std::string atom_name = BaseBuilder<Grammar>::createStringFromNode(it->children[1]);

	  const SignatureBySym& local_sig = boost::get<Tag::Sym>(*sig);
	  SignatureBySym::const_iterator loc_it = local_sig.find(atom_name);      

	  r->second.second->push_back(loc_it->localId);
	}
    }
}

} // namespace dmcs

#endif // LOCAL_KB_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
