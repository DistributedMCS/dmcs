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

#ifndef LOCAL_LOOP_FORMULA_BUILDER_H
#define LOCAL_LOOP_FORMULA_BUILDER_H

#include "LoopFormulaBuilder.h"
#include "LoopFormula.h"
#include "Theory.h"
#include "Signature.h"

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <list>


namespace dmcs {

///@brief another builder could be added that handles the case of global formulas if needed
class LocalLoopFormulaBuilder : public LoopFormulaBuilder
{
protected:
  typedef boost::subgraph<boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
						boost::property<boost::vertex_color_t, int>,
						boost::property<boost::edge_index_t, int> > > Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  
  
  Graph localDependencyGraph;
  const SignaturePtr sig;

  typedef std::vector<vertex_descriptor> Loop;
  typedef std::vector<Loop> Loops;

  void
  checkAllInducedSubGraphs(Loop possibleLoop, vertex_descriptor value,const RulesPtr& kb, const RulesPtr& br);

  void
  checkStronglyConnected(Loop& possibleLoop,const RulesPtr& kb, const RulesPtr& br);

  void
  createLocalLoopFormulae(Loop loop,const RulesPtr& kb, const RulesPtr& br);

  virtual void initialiseKappaDataStructure()= 0;
  virtual void fillKappaHead(std::size_t in) = 0;
  virtual void fillKappaPositiveBody(std::size_t in)= 0;
  virtual void fillKappaNegativeBody(std::size_t in) = 0;
  virtual void storeKappaDataStructure() = 0;
  virtual void createSupportFormula(Loop::const_iterator lbeg,
				    Loop::const_iterator lend,
				    std::vector<Rules::const_iterator> esr,
				    std::vector<Rules::const_iterator> sr)= 0;  

public:

  LocalLoopFormulaBuilder(const SignaturePtr& s, std::size_t size)
     : localDependencyGraph(size),
       sig(new Signature(*s))
  { }

  RulesPtr
  buildBridgeKappa(const BridgeRulesPtr& bridgeRules); 

  void
  createDependencyGraphAndKBKappa(const RulesPtr& kb);
    
  void
  buildLambda (const RulesPtr&, const RulesPtr&);

  const SignaturePtr&
  getSignature() const
  {
    return sig;
  }
};


} // namespace dmcs

#endif /* LOOPFORMULABUILDER_H */


// Local Variables:
// mode: C++
// End:

