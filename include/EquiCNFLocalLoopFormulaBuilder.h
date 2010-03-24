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
 * @file   EquiCNFLocalLoopFormulaBuilder.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sun Jan 19 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef EQUI_CNF_LOCAL_LOOP_FORMULA_BUILDER_H
#define EQUI_CNF_LOCAL_LOOP_FORMULA_BUILDER_H

#include "CNFLocalLoopFormulaBuilder.h"

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <list>
#include <sstream>

namespace dmcs {

class EquiCNFLocalLoopFormulaBuilder : public CNFLocalLoopFormulaBuilder
{

protected:  

  void
  createSupportFormula(Loop::const_iterator lbeg,
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

    resultingCNF = createEquiCNF(formula);    
    add(cnfFormula,resultingCNF);
  }
  
  TheoryPtr
  createEquiCNF(const TheoryPtr& DNF)
  {
    TheoryPtr result(new Theory);
    int counter = 0;

    for(Theory::const_iterator it = DNF->begin(); it != DNF->end(); ++it)
      {
	++counter;
	const int newVariableId = sig->size()+1;
	
	std::stringstream oss;
	oss << "$" << newVariableId;

	const std::string newVariableName = oss.str();
	sig->insert(Symbol(newVariableName,0,newVariableId,newVariableId));
	
	for(Clause::const_iterator jt = (*it)->begin(); jt != (*it)->end(); ++jt)
	  { 
	    ClausePtr newClause(new Clause);
	    	    
	    newClause->push_back(-1 * newVariableId);
	    newClause->push_back(*jt);
	    result->push_back(newClause);
	  } 
      }

    ClausePtr newClause(new Clause);
    for(std::size_t i = sig->size()-counter + 1; i <= sig->size(); ++i)
      {
	newClause->push_back(i);
      }
    result->push_back(newClause);
    
    return result;
  }


public:

  EquiCNFLocalLoopFormulaBuilder(const SignaturePtr& s, std::size_t size)
    : CNFLocalLoopFormulaBuilder(s,size)
  { }


};


} // namespace dmcs


#endif /* EQUI_CNF_LOOP_FORMULA_BUILDER_H */


// Local Variables:
// mode: C++
// End:
