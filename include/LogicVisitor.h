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
 * @file   LogicVisitor.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Fri Jan 15 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef LOGIC_VISITOR_H
#define LOGIC_VISITOR_H

#include "BaseVisitor.h"
#include "Context.h"
#include "Rule.h"

#include <iosfwd>


namespace dmcs {


class LogicVisitor : public BaseVisitor
{
protected:
  std::ostream& stream;

public:
  explicit
  LogicVisitor(std::ostream&);

  /**
   * returns the stream of the visitor.
   */
  std::ostream&
  getStream();

  void 
  visitRules(const ContextPtr& context);

  void 
  visitBridgeRules(const ContextPtr& context);

  void
  visitRule(const RulePtr& rule, const std::size_t contextId);

  void
  visitBridgeRule(const BridgeRulePtr& bridgeRule, const std::size_t contextId);

  virtual void
  printPositiveBridgeRule(std::ostream& os,const BridgeAtom& b);
  
  virtual void
  printNegativeBridgeRule(std::ostream& os,const BridgeAtom& b);

  void
  visitClause(const ClausePtr)
  { }

  void
  visitTheory(const TheoryPtr, std::size_t /* size */)
  { }
};


} // namespace dmcs

#endif // LOGIC_VISITOR_H

// Local Variables:
// mode: C++
// End:
