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
 * @file   LoopFormulaDirector.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sun Jan 3 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef LOOPFORMULADIRECTOR_H
#define LOOPFORMULADIRECTOR_H

#include "LoopFormulaBuilder.h"

#include <iostream>


namespace dmcs {


class LoopFormulaDirector
{
private:
  
  LoopFormulaBuilder* builder;
  
public:
  
  LoopFormulaDirector()
    : builder(0)
  { }


  void 
  setBuilder (LoopFormulaBuilder* b)
  { 
    builder = b;
  }


  void
  construct(RulesPtr kb, BridgeRulesPtr br)
  {
    assert(builder != 0);

#if defined(DEBUG)
    std::cerr << "creating graph and kappa" << std::endl;
#endif

    builder->createDependencyGraphAndKBKappa(kb);

#if defined(DEBUG)
    std::cerr << "creating bridge kappa" << std::endl;
#endif

    RulesPtr ref_br = builder->buildBridgeKappa(br);

#if defined(DEBUG)
    std::cerr << "creating lambda" << std::endl;
#endif

    builder->buildLambda(kb, ref_br);
  }

};


} // namespace dmcs

#endif /* LOOPFORMULADIRECTOR_H */


// Local Variables:
// mode: C++
// End:

