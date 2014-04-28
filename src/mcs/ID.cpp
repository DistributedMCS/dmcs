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
 * @file   ID.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * @date   Tue Dec  7 10:27:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/ID.h"
#include <boost/functional/hash.hpp>
#include <iomanip>

namespace dmcs {

std::size_t hash_value(const ID& id)
{
  std::size_t seed = 0;
  boost::hash_combine(seed, id.kind);
  boost::hash_combine(seed, id.address);
  return seed;
}

std::ostream& ID::print(std::ostream& o) const
{
  if( *this == ID_FAIL )
    return o << "ID_FAIL";

  o << "ID(0x" 
    << std::setfill('0') << std::hex << std::setw(8) << kind << "," 
    << std::setfill(' ') << std::dec << std::setw(4) << address;

  if( !!(kind & NAF_MASK) )
    o << " naf";

  const unsigned MAINKIND_MAX = 5;

  const char* mainkinds[MAINKIND_MAX] = {
    " atom",
    " term",
    " literal",
    " rule",
    " belief"
  };

  const unsigned mainkind = (kind & MAINKIND_MASK) >> MAINKIND_SHIFT;
  assert(mainkind < MAINKIND_MAX);
  o << mainkinds[mainkind];

  const unsigned SUBKIND_MAX = 11;

  const char* subkinds[MAINKIND_MAX][SUBKIND_MAX] = {
    { " ordinary_ground", " ordinary_nonground", " builtin",         " aggregate",   "",           "", " external", "", "", "", " module"},
    { " constant",        " integer",            " variable",        " builtin",     " predicate", "",              ""          },
    { " ordinary_ground", " ordinary_nonground", " builtin",         " aggregate",   "",           "", " external", "", "", "", " module"},
    { " regular",         " constraint",         " weak_constraint", " bridge_rule", "",           "", ""          },
    { " ",                "",                    "",                 "",             "",           "", "",          "", "" }
  };

  const unsigned subkind = (kind & SUBKIND_MASK) >> SUBKIND_SHIFT;
  assert(subkind < SUBKIND_MAX);
  assert(subkinds[mainkind][subkind][0] != 0);
  o << subkinds[mainkind][subkind];
  return o << ")";
}

// returns builtin term ID
// static
ID ID::termFromBuiltinString(const std::string& op)
{
  assert(!op.empty());
  switch(op.size())
  {
  case 1:
    switch(op[0])
    {
    case '=': return ID::termFromBuiltin(ID::TERM_BUILTIN_EQ);
    case '<': return ID::termFromBuiltin(ID::TERM_BUILTIN_LT);
    case '>': return ID::termFromBuiltin(ID::TERM_BUILTIN_GT);
    case '*': return ID::termFromBuiltin(ID::TERM_BUILTIN_MUL);
    case '+': return ID::termFromBuiltin(ID::TERM_BUILTIN_ADD);
    case '-': return ID::termFromBuiltin(ID::TERM_BUILTIN_SUB);
    case '/': return ID::termFromBuiltin(ID::TERM_BUILTIN_DIV);
    default: assert(false); return ID_FAIL;
    }
  case 2:
    if( op == "==" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_EQ);
    }
    else if( op == "!=" || op == "<>" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_NE);
    }
    else if( op == "<=" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_LE);
    }
    else if( op == ">=" )
    {
      return ID::termFromBuiltin(ID::TERM_BUILTIN_GE);
    }
    else
    {
      assert(false); return ID_FAIL;
    }
  }
  if( op == "#succ" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_SUCC); }
  else if( op == "#int" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_INT); }
  else if( op == "#count" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGCOUNT); }
  else if( op == "#min" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGMIN); }
  else if( op == "#max" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGMAX); }
  else if( op == "#sum" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGSUM); }
  else if( op == "#times" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGTIMES); }
  else if( op == "#avg" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGAVG); }
  else if( op == "#any" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_AGGANY); }
  else if( op == "#mod" )
  { return ID::termFromBuiltin(ID::TERM_BUILTIN_MOD); }
  else
  {
    assert(false);
    return ID_FAIL;
  }
}

namespace
{
  const char* builtinTerms[] =
    {
      "=",
      "!=",
      "<",
      "<=",
      ">",
      ">=",
      "*",
      "+",
      "-",
      "/",
      "#count",
      "#min",
      "#max",
      "#sum",
      "#times",
      "#avg",
      "#any",
      "#int",
      "#succ",
      "#mod",
    };
}

const char* ID::stringFromBuiltinTerm(IDAddress addr)
{
  assert(addr < (sizeof(builtinTerms)/sizeof(const char*)));
  return builtinTerms[addr];
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
