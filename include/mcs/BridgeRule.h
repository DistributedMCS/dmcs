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
 * @file   BridgeRule.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  12 17:45:20 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BRIDGE_RULE_H
#define BRIDGE_RULE_H

#include "mcs/ID.h"

namespace dmcs {

struct BridgeRule : private ostream_printable<BridgeRule>
{
  IDKind kind;
  ID head;
  Tuple body;

  BridgeRule(IDKind kind)
    : kind(kind), head(), body()
  { 
    assert(ID(kind,0).isBridgeRule()); 
  }

  BridgeRule(IDKind kind, ID head, const Tuple& body)
    : kind(kind), head(head), body(body)
  { 
    assert(ID(kind,0).isBridgeRule()); 
  }

  std::ostream& 
  print(std::ostream& o) const
  { 
    o << "Rule(" << head << " <- " << printvector(body);
    return o << ")"; 
  }
};

} // namespace dmcs

#endif // BRIDGE_RULE_H

// Local Variables:
// mode: C++
// End:
