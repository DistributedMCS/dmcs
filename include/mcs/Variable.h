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
 * @file   Variable.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Apr 22 08:24:40 2010
 * 
 * @brief  Variables Signature accessing and storing.
 * 
 * 
 */

#ifndef VARIABLE_H
#define VARIABLE_H

#include <iostream>
#include <limits>
#include <cassert>
#include <sstream>

// a ContextTerm is enconded as follows:
// 2 most signigicant bits for type (TYPE_RESERVE = 2), which can be
// + either a context id
// + or a context variable
// for the rest of the bits, 
// the higher half encodes the context id in which this variable appears

// in case of context variable:
// the lower half encodes the variable id 

// in the case of context id:
// the lower half encodes the context id of this term
typedef std::size_t ContextTerm;

// a SchematicBelief is encoded as follows:
// 2 most significant bits for type, which can be:
// + ordinary,
// + exact schematic,
// + or similar schematic
// the rest stores the assigned integer id of the belief (taken from the signature)
typedef std::size_t SchematicBelief;

#define IS_ORDINARY_BELIEF 0
#define IS_EXACT_SCHEMATIC_BELIEF 2
#define IS_SIMILAR_SCHEMATIC_BELIEF 3
#define IS_VARIABLE 1
#define TYPE_RESERVE 2

// all 1 bitset
#define MAX std::numeric_limits<ContextTerm>::max()

// it's very important that whenever using a bit operator, cast all integer constants to the respective type
// we had enough stupid bugs concerning 32- and 64-bit shifting
#define CONTEXT_TERM_SIZE sizeof(ContextTerm)*8
#define SCHEMATIC_BELIEF_SIZE sizeof(SchematicBelief)*8
#define SHIFT_HALF ((CONTEXT_TERM_SIZE - TYPE_RESERVE) / 2)
#define MASK_CONTEXT_VARIABLE (MAX >> TYPE_RESERVE)
#define MASK_VARIABLE_ID (MAX >> (TYPE_RESERVE + SHIFT_HALF))
#define MASK_CONTEXT_ID ((MAX >> TYPE_RESERVE) & (MAX << SHIFT_HALF))
#define MASK_BELIEF (MAX >> TYPE_RESERVE)
#define MASK_CONSTANT (MAX >> TYPE_RESERVE)


namespace dmcs {


// get the type of a schematic belief:
// - ordinary belief 
// - exact schematic belief
// - similar schematic belief
inline std::size_t
sBeliefType(SchematicBelief sb)
{
  return (sb >> (SCHEMATIC_BELIEF_SIZE - TYPE_RESERVE));
}


// set type for a schematic belief
inline SchematicBelief
sBeliefType(SchematicBelief sb, std::size_t type)
{
  return ((sb & MASK_BELIEF) | (type << (SCHEMATIC_BELIEF_SIZE - TYPE_RESERVE)));
}


// get the belief id
inline std::size_t
sBelief(SchematicBelief sb)
{
  return (sb & MASK_BELIEF);
}


// set the belief id
inline SchematicBelief
sBelief(SchematicBelief sb, std::size_t b)
{
  return ((sb & (~MASK_BELIEF)) | b);
}


// set up a schematic belief with 2 components:
// type and schematic constant
inline SchematicBelief
constructSchematicBelief(std::size_t type, std::size_t schematic_constant)
{
  return sBeliefType(sBelief(0, schematic_constant), type);
}


// check whether a context term is a variable
inline bool
isCtxVar(ContextTerm num)
{
  return ((num >> (CONTEXT_TERM_SIZE - TYPE_RESERVE)) & IS_VARIABLE);
}


// set a context term to be variable
inline ContextTerm
ctxVar(ContextTerm num)
{
  return num | ((ContextTerm)1 << (CONTEXT_TERM_SIZE - TYPE_RESERVE));
}


// get the context id part of the context term
inline std::size_t
ctxID(const ContextTerm num)
{
  assert(isCtxVar(num));
  return (num & MASK_CONTEXT_VARIABLE) >> SHIFT_HALF;
}


// set the context id part of the context term
inline ContextTerm
ctxID(ContextTerm num, std::size_t ctx)
{
  return (num & (~MASK_CONTEXT_ID)) | (ctx << SHIFT_HALF);
}


// get variable part of the context term
inline std::size_t
varID(const ContextTerm num)
{
  assert(isCtxVar(num));
  return num & MASK_VARIABLE_ID;
}


// set variable part of the context term
inline ContextTerm
varID(ContextTerm num, std::size_t var)
{
  return ctxVar((num & (~MASK_VARIABLE_ID)) | var);
}


// set a context variable term
inline ContextTerm
ctxVarTerm(std::size_t contextID, std::size_t variableID)
{
  return varID(ctxID(0, contextID), variableID);
}


// set a context constant term
inline ContextTerm
ctxConstTerm(std::size_t context_const)
{
  assert(!isCtxVar(context_const));
  return context_const & MASK_CONSTANT;
}


///@todo: check how to overload operator<< of an interger in a different format
inline std::string
ctx2string(const ContextTerm num)
{
  std::string tmp;

  tmp = "[";
  std::stringstream out;

  if (isCtxVar(num))
    {
      std::size_t id = ctxID(num);
      std::size_t var = varID(num);
      out << "v, " << id << ", " << var;
      
    }
  else
    {
      out << "c," << num;
    }

  tmp = tmp + out.str() + "]";

  return tmp;
}

inline std::string
sb2string(const SchematicBelief sb)
{
  std::stringstream out;

  std::size_t type = sBeliefType(sb);
  std::size_t sbelief = sBelief(sb);

  out << "[" << type << ", " << sbelief << "]";

  return out.str();
}

} // namespace dmcs

#endif

// Local Variables:
// mode: C++
// End:
