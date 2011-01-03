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
 * @file   LoopFormulaBuilder.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sun Jan 3 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef LOOPFORMULABUILDER_H
#define LOOPFORMULABUILDER_H

#include "mcs/Rule.h"
#include "mcs/Theory.h"


namespace dmcs {


class LoopFormulaBuilder
{

public:

  virtual RulesPtr
  buildBridgeKappa(const BridgeRulesPtr& bridgeRules) = 0; 

  virtual void
  createDependencyGraphAndKBKappa(const RulesPtr& kb) = 0;
    
  virtual void
  buildLambda (const RulesPtr&, const RulesPtr&) = 0;
};


} // namespace dmcs

#endif /* LOOPFORMULABUILDER_H */


// Local Variables:
// mode: C++
// End:
