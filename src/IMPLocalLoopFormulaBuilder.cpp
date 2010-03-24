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
 * @file   LocalLoopFormulaBuilder.cpp
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sun Jan 3 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "IMPLocalLoopFormulaBuilder.h"
#include "LoopFormula.h"
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/graph/strong_components.hpp>

using namespace dmcs;


void
IMPLocalLoopFormulaBuilder::initialiseKappaDataStructure()
{
  KappaImplicationPtr i(new KappaImplication);
  imp = i;
}

void
IMPLocalLoopFormulaBuilder::fillKappaHead(std::size_t in)
{
  imp->consequent.push_back(in);
}

void 
IMPLocalLoopFormulaBuilder::fillKappaPositiveBody(std::size_t in)
{
  imp->positiveAntecedent.push_back(in);
}
void 
IMPLocalLoopFormulaBuilder::fillKappaNegativeBody(std::size_t in)
{
  imp->negativeAntecedent.push_back(in);
}

void 
IMPLocalLoopFormulaBuilder::storeKappaDataStructure()
{
  kappa.push_back(imp);
}

void 
IMPLocalLoopFormulaBuilder::createSupportFormula(Loop::const_iterator lbeg,
						 Loop::const_iterator lend,
						 std::vector<Rules::const_iterator> esr,
						 std::vector<Rules::const_iterator> sr)
{
  LambdaImplicationPtr imp(new LambdaImplication);  
  supportFormula(lbeg, lend, esr.begin(), esr.end(), imp->consequent);
  supportFormula(lbeg, lend, sr.begin(), sr.end(), imp->consequent);
  imp->antecedent.insert(imp->antecedent.begin(), lbeg, lend);
  loopFormulae.push_back(imp);
}
