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
 * @file   ID.h
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * @date   Tue Dec  6 16:20:24 2011
 * 
 * @brief  
 * 
 * 
 */


#ifndef ID_H
#define ID_H

#include <boost/cstdint.hpp>
#include "mcs/Printhelpers.h"

namespace dmcs {

typedef uint32_t IDKind;
typedef uint32_t IDAddress;

struct ID : private ostream_printable<ID>
{
  IDKind kind;
  IDAddress address;

  ID()
    : kind(ALL_ONES), address(ALL_ONES) 
  { }

  ID(IDKind kind, IDAddress address)
    : kind(kind), address(address) 
  { }
  // no virtual here!
  // this struct must fit into an uint64_t and have no vtable!
  
  static const uint32_t ALL_ONES =             0xFFFFFFFF;
  
  static const IDKind NAF_MASK =               0x80000000;
  static const IDKind MAINKIND_MASK =          0x70000000;
  static const uint8_t MAINKIND_SHIFT =        28;
  static const IDKind SUBKIND_MASK =           0x0F000000;
  static const uint8_t SUBKIND_SHIFT =         24;
  static const IDKind PROPERTY_MASK =          0x00FF0000;
  static const uint8_t PROPERTY_SHIFT =        16;
  static const IDKind CONTEXT_ID_MASK =        0x0000FFFF;
  
  static const IDKind MAINKIND_ATOM =          0x00000000;
  static const IDKind MAINKIND_TERM =          0x10000000;
  static const IDKind MAINKIND_LITERAL =       0x20000000;
  static const IDKind MAINKIND_RULE =          0x30000000;
  static const IDKind MAINKIND_BELIEF =        0x40000000;
  
  static const IDKind SUBKIND_TERM_CONSTANT =  0x00000000;
  static const IDKind SUBKIND_TERM_INTEGER =   0x01000000;
  static const IDKind SUBKIND_TERM_VARIABLE =  0x02000000;
  static const IDKind SUBKIND_TERM_BUILTIN =   0x03000000;
  static const IDKind SUBKIND_TERM_PREDICATE = 0x04000000;
  
  static const IDKind SUBKIND_ATOM_ORDINARYG = 0x00000000;
  static const IDKind SUBKIND_ATOM_ORDINARYN = 0x01000000;
  static const IDKind SUBKIND_ATOM_BUILTIN =   0x02000000;
  static const IDKind SUBKIND_ATOM_AGGREGATE = 0x03000000;
  // 6 and A -> check -> because of method isOrdinaryAtom that masked ID with builtin != builtin
  static const IDKind SUBKIND_ATOM_EXTERNAL =  0x06000000;
  static const IDKind SUBKIND_ATOM_MODULE =    0x0A000000;
  
  static const IDKind SUBKIND_RULE_REGULAR =        0x00000000;
  static const IDKind SUBKIND_RULE_CONSTRAINT =     0x01000000;
  static const IDKind SUBKIND_RULE_WEAKCONSTRAINT = 0x02000000;
  static const IDKind SUBKIND_RULE_BRIDGE_RULE =    0x03000000;
  
  //                                             0x00FF0000
  static const IDKind PROPERTY_VAR_ANONYMOUS   = 0x00010000;
  static const IDKind PROPERTY_RULE_EXTATOMS   = 0x00080000;
  static const IDKind PROPERTY_RULE_DISJ       = 0x00100000;
  static const IDKind PROPERTY_RULE_MODATOMS   = 0x00400000;
  static const IDKind PROPERTY_RULE_UNMODATOMS = 0xFFBFFFFF;
  static const IDKind PROPERTY_AUX             = 0x00800000;
  
  // for builtin terms, this is the address part (no table)
  // beware: must be synchronized with isInfixBuiltin() and builtinTerms[]
  enum TermBuiltinAddress
  {
    // first we have the infix builtins (see isInfixBuiltin)
    TERM_BUILTIN_EQ,
    TERM_BUILTIN_NE,
    TERM_BUILTIN_LT,
    TERM_BUILTIN_LE,
    TERM_BUILTIN_GT,
    TERM_BUILTIN_GE,
    TERM_BUILTIN_MUL,
    TERM_BUILTIN_ADD,
    TERM_BUILTIN_SUB,
    TERM_BUILTIN_DIV,
    // then the prefix builtins (see isInfixBuiltin)
    TERM_BUILTIN_AGGCOUNT,
    TERM_BUILTIN_AGGMIN,
    TERM_BUILTIN_AGGMAX,
    TERM_BUILTIN_AGGSUM,
    TERM_BUILTIN_AGGTIMES,
    TERM_BUILTIN_AGGAVG,
    TERM_BUILTIN_AGGANY,
    TERM_BUILTIN_INT,
    TERM_BUILTIN_SUCC,
    TERM_BUILTIN_MOD,
  };
  
  static inline ID termFromInteger(uint32_t i)
  { 
    return ID(ID::MAINKIND_TERM | ID::SUBKIND_TERM_INTEGER, i); 
  }

  static inline ID termFromBuiltin(TermBuiltinAddress b)
  { 
    return ID(ID::MAINKIND_TERM | ID::SUBKIND_TERM_BUILTIN, b); 
  }

  static ID termFromBuiltinString(const std::string& op);
  static const char* stringFromBuiltinTerm(IDAddress addr);

  static inline ID posLiteralFromAtom(ID atom)
  { 
    assert(atom.isAtom()); 
    return ID(atom.kind | MAINKIND_LITERAL, atom.address); 
  }

  static inline ID nafLiteralFromAtom(ID atom)
  { 
    assert(atom.isAtom()); 
    return ID(atom.kind | MAINKIND_LITERAL | NAF_MASK, atom.address); 
  }

  static inline ID literalFromAtom(ID atom, bool naf)
  { 
    assert(atom.isAtom()); 
    return (naf?nafLiteralFromAtom(atom):posLiteralFromAtom(atom)); 
  }

  static inline ID atomFromLiteral(ID literal)
  { 
    assert(literal.isLiteral());
    return ID((literal.kind & (~(NAF_MASK|MAINKIND_MASK))) | MAINKIND_ATOM, literal.address); 
  }

  static inline ID beliefFromCtxIdAddress(uint16_t ctx_id, IDAddress address)
  {
    return ID(ID::MAINKIND_BELIEF | ctx_id, address);
  }

  inline bool isTerm() const          
  { 
    return (kind & MAINKIND_MASK) == MAINKIND_TERM; 
  }

  inline bool isConstantTerm() const  
  { 
    assert(isTerm()); 
    return (kind & SUBKIND_MASK) == SUBKIND_TERM_CONSTANT; 
  }

  inline bool isIntegerTerm() const
  { 
    assert(isTerm()); 
    return (kind & SUBKIND_MASK) == SUBKIND_TERM_INTEGER; 
  }

  inline bool isVariableTerm() const  
  { 
    assert(isTerm()); 
    return (kind & SUBKIND_MASK) == SUBKIND_TERM_VARIABLE; 
  }

  inline bool isBuiltinTerm() const   
  { 
    assert(isTerm()); 
    return (kind & SUBKIND_MASK) == SUBKIND_TERM_BUILTIN; 
  }

  inline bool isPredicateTerm() const   
  { 
    assert(isTerm()); 
    return (kind & SUBKIND_MASK) == SUBKIND_TERM_PREDICATE; 
  }

  inline bool isAtom() const
  { 
    return (kind & MAINKIND_MASK) == MAINKIND_ATOM; 
  }

  // true for ground or nonground ordinary atoms
  // (special bit trick)
  inline bool isOrdinaryAtom() const
  { 
    assert(isAtom() || isLiteral()); 
    return (kind & SUBKIND_ATOM_BUILTIN) != SUBKIND_ATOM_BUILTIN; 
  }

  inline bool isOrdinaryGroundAtom() const
  { 
    assert(isAtom() || isLiteral()); 
    return !(kind & SUBKIND_MASK); 
  }

  inline bool isOrdinaryNongroundAtom() const
  { 
    assert(isAtom() || isLiteral()); 
    return (kind & SUBKIND_MASK) == SUBKIND_ATOM_ORDINARYN; 
  }

  inline bool isBuiltinAtom() const
  { 
    assert(isAtom() || isLiteral()); 
    return (kind & SUBKIND_MASK) == SUBKIND_ATOM_BUILTIN; 
  }

  inline bool isAggregateAtom() const 
  { 
    assert(isAtom() || isLiteral()); 
    return (kind & SUBKIND_MASK) == SUBKIND_ATOM_AGGREGATE; 
  }

  inline bool isExternalAtom() const
  { 
    assert(isAtom() || isLiteral()); 
    return (kind & SUBKIND_MASK) == SUBKIND_ATOM_EXTERNAL; 
  }

  inline bool isModuleAtom() const    
  { 
    assert(isAtom() || isLiteral()); 
    return (kind & SUBKIND_MASK) == SUBKIND_ATOM_MODULE; 
  }

  inline bool isLiteral() const
  { 
    return (kind & MAINKIND_MASK) == MAINKIND_LITERAL; 
  }

  inline bool isBelief() const
  {
    return (kind & MAINKIND_MASK) == MAINKIND_BELIEF;
  }

  inline bool isNaf() const
  { 
    return (kind & NAF_MASK) == NAF_MASK; 
  }

  inline bool isAuxiliary() const
  { 
    return (kind & PROPERTY_AUX) == PROPERTY_AUX; 
  }
  
  inline bool isRule() const
  { 
    return (kind & MAINKIND_MASK) == MAINKIND_RULE; 
  }

  inline bool isRegularRule() const   
  { 
    assert(isRule()); 
    return (kind & SUBKIND_MASK) == SUBKIND_RULE_REGULAR; 
  }

  inline bool isConstraint() const
  { 
    assert(isRule()); 
    return (kind & SUBKIND_MASK) == SUBKIND_RULE_CONSTRAINT; 
  }

  inline bool isWeakConstraint() const
  { 
    assert(isRule()); 
    return (kind & SUBKIND_MASK) == SUBKIND_RULE_WEAKCONSTRAINT; 
  }

  inline bool isBridgeRule() const
  {
    assert( isRule() );
    return (kind & SUBKIND_MASK) == SUBKIND_RULE_BRIDGE_RULE;
  }

  inline bool doesRuleContainExtatoms() const
  { 
    assert(isRule()); 
    return (kind & PROPERTY_RULE_EXTATOMS) == PROPERTY_RULE_EXTATOMS; 
  }

  inline bool doesRuleContainModatoms() const
  { 
    assert(isRule()); 
    return (kind & PROPERTY_RULE_MODATOMS) == PROPERTY_RULE_MODATOMS; 
  }

  inline bool isRuleDisjunctive() const 
  { 
    assert(isRule()); 
    return (kind & PROPERTY_RULE_DISJ) == PROPERTY_RULE_DISJ; 
  }

  inline bool isAnonymousVariable() const 
  { 
    assert(isVariableTerm()); 
    return (kind & PROPERTY_VAR_ANONYMOUS) == PROPERTY_VAR_ANONYMOUS; 
  }

  inline std::size_t
  contextID()
  {
    assert ( isBelief() );
    return (kind & CONTEXT_ID_MASK);
  }

  inline bool operator==(const ID& id2) const { return kind == id2.kind && address == id2.address; }
  inline bool operator!=(const ID& id2) const { return kind != id2.kind || address != id2.address; }
  inline ID operator|(const ID& id2) const    { return ID(kind | id2.kind, address | id2.address); }
  inline ID operator&(const ID& id2) const    { return ID(kind & id2.kind, address & id2.address); }
  inline operator uint64_t() const            { return *reinterpret_cast<const uint64_t*>(this); }
  
  std::ostream& print(std::ostream& o) const;
};

std::size_t hash_value(const ID& id);

const ID ID_FAIL(ID::ALL_ONES, ID::ALL_ONES);

typedef std::vector<ID> Tuple;

} // namespace dmcs

#endif // ID_H

// Local Variables:
// mode: C++
// End:
