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
 * @file   CnfBuilder.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:47:20 2009
 * @deprecated not used anymore...
 * 
 * @brief  
 * 
 * 
 */

#if !defined(CNF_BUILDER_TCC)
#define CNF_BUILDER_TCC

#include <boost/functional/hash.hpp>

namespace dmcs {

template<typename Grammar>
CnfBuilder<Grammar>::CnfBuilder(TheoryPtr t, BiMapId2Name& i)
  : theory(t),
    id2name(i)
{
}


template<typename Grammar>
void
CnfBuilder<Grammar>::buildNode(typename BaseBuilder<Grammar>::node_t& node)
{
  assert (node.value.id() == Grammar::Clause);

  ClausePtr c(new Clause);

  for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
       it != node.children.end(); ++it)
    {
      build_atom(*it, c);
    }

  theory->push_back(c);
}


template<typename Grammar>
void
CnfBuilder<Grammar>::build_atom(typename BaseBuilder<Grammar>::node_t& node, ClausePtr clause)
{
  assert(node.value.id() == Grammar::Atom);

  if (node.children.size() == 2)
    {
      add_atom(-1, BaseBuilder<Grammar>::createStringFromNode(node.children[1]), clause);
    }
  else
    {
      add_atom(1, BaseBuilder<Grammar>::createStringFromNode(node.children[0]), clause);
    }
}


template<typename Grammar>
void 
CnfBuilder<Grammar>::add_atom (int sign, const std::string& atom_name, ClausePtr clause)
{
  boost::hash<std::string> string_hash;
  int atom_hash = std::abs((int)string_hash(atom_name));
  BiMapId2Name::value_type p(atom_hash, atom_name);
  id2name.insert(p);
  clause->push_back(sign * atom_hash);
}


template<typename Grammar>
BiMapId2Lid
CnfBuilder<Grammar>::getLocalSATId() const
{
#if defined(DEBUG)
  std::cerr << "Create Local SAT ID" << std::endl;
#endif //DEBUG

  BiMapId2Lid sat_id2lid;

  int i = 1;
  for (BiMapId2Name::left_const_iterator it = id2name.left.begin(); 
       it != id2name.left.end(); ++i, ++it)
    {
      BiMapId2Lid::value_type p(it->first, i);
      sat_id2lid.insert(p);
    }

  return sat_id2lid;
}


} // namespace dmcs

#endif // _CNF_BUILDER_TCC

// Local Variables:
// mode: C++
// End:
