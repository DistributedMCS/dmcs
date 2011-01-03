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

#ifndef IMP_LOCAL_LOOP_FORMULA_BUILDER_H
#define IMP_LOCAL_LOOP_FORMULA_BUILDER_H

#include "loopformula/LocalLoopFormulaBuilder.h"
#include "loopformula/LoopFormula.h"
#include "mcs/Theory.h"
#include "mcs/Signature.h"

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <list>


namespace dmcs {

///@brief another builder could be added that handles the case of global formulas if needed
class IMPLocalLoopFormulaBuilder : public LocalLoopFormulaBuilder
{
protected:
  
  struct KappaImplication
  {
    std::list<int> positiveAntecedent; /// should be treated as conjunction of atoms
    std::list<int> negativeAntecedent; /// should be treated as conjunction of negative atoms
    std::list<int> consequent; /// should be treated as disjunction of atoms
  };
  typedef boost::shared_ptr<KappaImplication> KappaImplicationPtr;

  struct LambdaImplication
  {
    Loop antecedent; /// should be treated as disjunction of atoms
    std::list<EpsilonConjunction> consequent ; /// should be treated as disjunction of atoms
  };
  typedef boost::shared_ptr<LambdaImplication> LambdaImplicationPtr;

  std::list<KappaImplicationPtr> kappa; /// conjunction of all Kappa formulas
  std::list<LambdaImplicationPtr> loopFormulae; /// conjunction of all loop formula equations

  KappaImplicationPtr imp;

protected:
  void initialiseKappaDataStructure();
  void fillKappaHead(std::size_t in);
  void fillKappaPositiveBody(std::size_t in);
  void fillKappaNegativeBody(std::size_t in);
  void storeKappaDataStructure();
  void createSupportFormula(Loop::const_iterator lbeg,
			    Loop::const_iterator lend,
			    std::vector<Rules::const_iterator> esr,
			    std::vector<Rules::const_iterator> sr);  


public:

   IMPLocalLoopFormulaBuilder(const SignaturePtr& s, std::size_t size)
     : LocalLoopFormulaBuilder (s,size)
  { }

  // add sth to return values if needed
};


} // namespace dmcs

#endif /* LOOPFORMULABUILDER_H */


// Local Variables:
// mode: C++
// End:
