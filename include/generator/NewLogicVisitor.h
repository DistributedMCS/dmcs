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
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Oct 29 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef __NEW_LOGIC_VISITOR_H__
#define __NEW_LOGIC_VISITOR_H__

#include "generator/BaseVisitor.h"
#include "mcs/BeliefTable.h"
#include "mcs/Rule.h"

#include <iosfwd>


namespace dmcs {


class NewLogicVisitor : public BaseVisitor
{
protected:
  std::ostream& stream;

public:
  NewLogicVisitor(std::ostream&);

  /**
   * returns the stream of the visitor.
   */
  std::ostream&
  getStream();

  void
  visitRule(const RulePtr& rule, const std::size_t contextId, const BeliefTablePtr& sigma);

  void
  visitBridgeRule(const BridgeRulePtr& bridgeRule, const std::size_t contextId, const BeliefTableVecPtr& sigma_vec);

  virtual void
  printPositiveBridgeRule(std::ostream& os,const BridgeAtom& b, const BeliefTableVecPtr& sigma_vec);
  
  virtual void
  printNegativeBridgeRule(std::ostream& os,const BridgeAtom& b, const BeliefTableVecPtr& sigma_vec);

private:
  std::string
  atom_name(const std::size_t contextId, const std::size_t atom_id, const BeliefTablePtr& sigma);
};


} // namespace dmcs

#endif // __NEW_LOGIC_VISITOR_H__

// Local Variables:
// mode: C++
// End:
