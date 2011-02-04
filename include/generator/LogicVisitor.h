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

#include "generator/BaseVisitor.h"
#include "dmcs/Context.h"
#include "mcs/Rule.h"

#include <iosfwd>


namespace dmcs {


class LogicVisitor : public BaseVisitor
{
protected:
  std::ostream& stream;

public:
  LogicVisitor(std::ostream&);

  /**
   * returns the stream of the visitor.
   */
  std::ostream&
  getStream();

  void 
  visitRules(const ContextPtr& context, const SignatureVecPtr& sigmas);

  void 
  visitBridgeRules(const ContextPtr& context, const SignatureVecPtr& sigmas);

  void
  visitRule(const RulePtr& rule, const std::size_t contextId, const SignatureVecPtr& sigmas);

  void
  visitBridgeRule(const BridgeRulePtr& bridgeRule, const std::size_t contextId, const SignatureVecPtr& sigmas);

  virtual void
  printPositiveBridgeRule(std::ostream& os,const BridgeAtom& b, const SignatureVecPtr& sigmas);
  
  virtual void
  printNegativeBridgeRule(std::ostream& os,const BridgeAtom& b, const SignatureVecPtr& sigmas);

};


} // namespace dmcs

#endif // LOGIC_VISITOR_H

// Local Variables:
// mode: C++
// End:
