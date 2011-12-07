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
 * @file   Atoms.hpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Storage classes for atoms: Atom, OrdinaryAtom, BuiltinAtom, AggregateAtom, BridgeAtom
 */

#ifndef ATOMS_H
#define ATOMS_H

#include "dlvhex/ID.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <vector>
#include <list>

namespace dmcs {

struct Atom
{
  // the kind part of the ID of this atom
  IDKind kind;

  // the ID representation of the main tuple of this atom
  // (for builtin and ordinary atoms, the main tuple is the only content)
  // (aggregate atoms add an "inner tuple" for the aggregate conditions)
  // (external atoms add an "input tuple" for the inputs)
  Tuple tuple;

  // used for indices
  ID front() const 
  { 
    return tuple.front(); 
  }

protected:
  // atom should not be used directly, so no public constructor
  Atom(IDKind kind)
    : kind(kind), tuple()
  { 
    assert(ID(kind,0).isAtom()); 
  }

  Atom(IDKind kind, const Tuple& tuple)
    : kind(kind), tuple(tuple)
  { 
    assert(ID(kind,0).isAtom()); 
  }
};

// regarding strong negation:
// during the parse process we do the following:
// we convert strong negation -<foo> into <foo'> (careful with variables in <foo>!)
// we add constraint :- <foo>, <foo'>.
// we somehow mark the <foo'> as strongly negated helper s.t. output can correctly print results
//
// for the first implementation, we leave out strong negation alltogether (not parseable)
struct OrdinaryAtom : public Atom, private ostream_printable<OrdinaryAtom>
{
  // the textual representation of the whole thing
  // this is stored for efficient parsing and printing
  // @todo make this a template parameter of OrdinaryAtom, so that we can store various "efficient" representations here (depending on the solver dlvhex should work with; e.g., we could store clasp- or dlv-library internal atom representations here and index them) if we don't need it, we can replace it by an empty struct and conserve space
  // TODO if we get answer sets in a structured way we do not need to parse them anymore
  // TODO if we only need this for printing, we should generate it on-demand and save a lot of effort
  std::string text;

  bool unifiesWith(const OrdinaryAtom& a) const;

  OrdinaryAtom(IDKind kind)
    : Atom(kind), text()
  { 
    assert(ID(kind,0).isOrdinaryAtom()); 
  }

  OrdinaryAtom(IDKind kind, const std::string& text)
    : Atom(kind), text(text)
  { 
    assert(ID(kind,0).isOrdinaryAtom()); 
    assert(!text.empty()); 
  }

  OrdinaryAtom(IDKind kind, const std::string& text, const Tuple& tuple)
    : Atom(kind, tuple), text(text)
  { 
    assert(ID(kind,0).isOrdinaryAtom());
    assert(!text.empty()); 
  }
  
  std::ostream& print(std::ostream& o) const
  { 
    return o << "OrdinaryAtom(" << std::hex << kind << std::dec << ",'" << text << "'," << printvector(tuple) << ")"; }

};

struct BuiltinAtom : public Atom, private ostream_printable<BuiltinAtom>
{
  // for ternary builtins of the form (A = B * C) tuple contains
  // in this order: <*, B, C, A>
  BuiltinAtom(IDKind kind)
    : Atom(kind)
  { 
    assert(ID(kind,0).isBuiltinAtom()); 
  }
  
  BuiltinAtom(IDKind kind, const Tuple& tuple)
    : Atom(kind, tuple)
  { 
    assert(ID(kind,0).isBuiltinAtom()); 
  }
  
  std::ostream& print(std::ostream& o) const
  { 
    return o << "BuiltinAtom(" << printvector(tuple) << ")"; 
  }
};

struct AggregateAtom : public Atom, private ostream_printable<AggregateAtom>
{
  // Atom::tuple is used for outer conditions (always contains 5 elements):
  // tuple[0] = left term or ID_FAIL
  // tuple[1] = left comparator or ID_FAIL
  // tuple[2] = aggregation function
  // tuple[3] = right comparator or ID_FAIL
  // tuple[4] = right term or ID_FAIL

  // variables of the symbolic set
  Tuple variables;

  // atoms in conjunction of the symbolic set
  #warning TODO rename this from atoms to literals, as this might contain literals
  Tuple atoms;

  AggregateAtom(IDKind kind)
    : Atom(kind, Tuple(5, ID_FAIL)), variables(), atoms()
  { 
    assert(ID(kind,0).isAggregateAtom()); 
  }

  AggregateAtom(IDKind kind, const Tuple& tuple, const Tuple& variables, const Tuple& atoms)
    : Atom(kind, tuple), variables(variables), atoms(atoms)
  { 
    assert(ID(kind,0).isAggregateAtom()); 
    assert(tuple.size() == 5);
    assert(!variables.empty()); assert(!atoms.empty()); 
  }

  std::ostream& print(std::ostream& o) const
  { 
    return o << "AggregateAtom(" << printvector(tuple) 
	     << " with vars " << printvector(variables) 
	     << " and literals " << printvector(atoms) << ")"; 
  }
};

} // namespace dmcs

#endif // ATOMS_H

// Local Variables:
// mode: C++
// End:
