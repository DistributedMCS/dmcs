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
 * @date   Sun Jan 9 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif


#include "loopformula/CNFLocalLoopFormulaBuilder.h"

#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/graph/strong_components.hpp>


using namespace dmcs;


void
CNFLocalLoopFormulaBuilder::initialiseKappaDataStructure()
{
  ClausePtr c(new Clause);      
  kappaDisjunction = c;
}

void
CNFLocalLoopFormulaBuilder::fillKappaHead(std::size_t in)
{
  kappaDisjunction->push_back(in);
}

void 
CNFLocalLoopFormulaBuilder::fillKappaPositiveBody(std::size_t in)
{
  kappaDisjunction->push_back(-1 * in);
}

void 
CNFLocalLoopFormulaBuilder::fillKappaNegativeBody(std::size_t in)
{
  kappaDisjunction->push_back(in);
}

void 
CNFLocalLoopFormulaBuilder::storeKappaDataStructure()
{
  add(cnfFormula,kappaDisjunction);
}

void 
CNFLocalLoopFormulaBuilder::createSupportFormula(Loop::const_iterator lbeg,
						 Loop::const_iterator lend,
						 std::vector<Rules::const_iterator> esr,
						 std::vector<Rules::const_iterator> sr)
{
  TheoryPtr formula(new Theory);
  ClausePtr loop = createNegatedConjunction(lbeg,lend);
  add(formula,loop);

  supportFormula(lbeg, lend, esr.begin(), esr.end(), formula);
  supportFormula(lbeg, lend, sr.begin(), sr.end(), formula);
  ClausePtr empty(new Clause);
  TheoryPtr resultingCNF(new Theory);
  resultingCNF = createCNF(formula, empty);    
  add(cnfFormula,resultingCNF);

}
