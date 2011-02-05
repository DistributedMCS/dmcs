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
 * @file   CNFLocalLoopFormulaBuilder.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sun Jan 9 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef CNF_LOCAL_LOOP_FORMULA_BUILDER_H
#define CNF_LOCAL_LOOP_FORMULA_BUILDER_H

#include "LocalLoopFormulaBuilder.h"
#include "LoopFormula.h"

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <list>


namespace dmcs {


class CNFLocalLoopFormulaBuilder : public LocalLoopFormulaBuilder
{
protected:
  
  TheoryPtr cnfFormula;
  ClausePtr kappaDisjunction;      

protected:  
  void initialiseKappaDataStructure();
  void fillKappaHead(std::size_t in);
  void fillKappaPositiveBody(std::size_t in);
  void fillKappaNegativeBody(std::size_t in);
  void storeKappaDataStructure();
  virtual void createSupportFormula(Loop::const_iterator lbeg,
			    Loop::const_iterator lend,
			    std::vector<Rules::const_iterator> esr,
			    std::vector<Rules::const_iterator> sr);  

public:

  CNFLocalLoopFormulaBuilder(std::size_t size,std::size_t placement)
    :LocalLoopFormulaBuilder(size,placement),
     cnfFormula(new Theory)
  { }
  
  TheoryPtr &
  getFormula()
  {
    return cnfFormula;
  }

};


} // namespace dmcs

#endif /* CNF_LOOP_FORMULA_BUILDER_H */


// Local Variables:
// mode: C++
// End:
