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


#include "loopformula/LocalLoopFormulaBuilder.h"
#include "loopformula/LoopFormula.h"
#include <iostream>
#include <functional>
#include <iostream>
#include <algorithm>
#include <iterator>


#include <boost/shared_ptr.hpp>
#include <boost/graph/strong_components.hpp>


using namespace dmcs;


///@todo should be done in-place
RulesPtr
LocalLoopFormulaBuilder::buildBridgeKappa(const BridgeRulesPtr& bridgeRules) 
{
  RulesPtr res(new Rules);
  
  for (BridgeRules::const_iterator it = bridgeRules->begin(); it != bridgeRules->end(); ++it)
    {
      RulePtr result(new Rule);
      HeadPtr h(new Head);
      PositiveBodyPtr p(new PositiveBody);
      NegativeBodyPtr n (new NegativeBody);

      initialiseKappaDataStructure();

      result->first = h;
      result->second.first = p;
      result->second.second = n;

      const Head& head = getHead(*it);
      for (Head::const_iterator jt = head.begin(); jt != head.end(); ++jt)
	{
	  assert(*jt > 0);
	  (result->first)->push_back(*jt);
	  fillKappaHead(*jt);
	}
      
      // postive body
      const PositiveBridgeBody& pb = getPositiveBody(*it);  
      for (PositiveBridgeBody::const_iterator jt = pb.begin();jt != pb.end(); ++jt)
	{
	  assert(jt->second > 0);
	  (result->second.first)->push_back(jt->second);
	  fillKappaPositiveBody(jt->second);
	}
      
      // negative body
      const NegativeBridgeBody& nb = getNegativeBody(*it);
      for (NegativeBridgeBody::const_iterator jt = nb.begin(); jt != nb.end(); ++jt)
	{
	  assert(jt->second > 0);
	  (result->second.second)->push_back(jt->second);
	  fillKappaNegativeBody(jt->second);
	}
      res->push_back(result);
      storeKappaDataStructure();
    }
  return res;
}


void
LocalLoopFormulaBuilder::createDependencyGraphAndKBKappa(const RulesPtr& kb)
{
  // add positive kb dependencies to the graph
  for (Rules::const_iterator it = kb->begin(); it != kb->end(); ++it)
    {

      initialiseKappaDataStructure();

      bool createdPbody = false;      
      const Head& h = getHead(*it);

      for (Head::const_iterator jt = h.begin(); jt != h.end(); ++jt)
	{
	  assert(*jt > 0);
	  fillKappaHead(*jt);
	  for (PositiveBody::const_iterator kt = (*it)->second.first->begin(); kt != (*it)->second.first->end(); ++kt)
	    {
	      boost::add_edge((*jt-1), (*kt-1), localDependencyGraph);            
	      if (!createdPbody)
		{
		  assert(*kt > 0);
		  fillKappaPositiveBody(*kt);
		}
	    }
	  createdPbody = true;
	}

      for (NegativeBody::iterator kt = (*it)->second.second->begin(); kt != (*it)->second.second->end(); ++kt)
	{
	  assert(*kt > 0);
	  fillKappaNegativeBody(*kt);
	}
      storeKappaDataStructure();
    }
}


void
LocalLoopFormulaBuilder::checkStronglyConnected(Loop& possibleLoop,
						const RulesPtr& kb, 
						const RulesPtr& br)
{
  Graph *G = 0;

  // this if condition is needed, otherwise a segmentation fault occurs
  if (possibleLoop.size() == boost::num_vertices(localDependencyGraph))
    {
      G = &localDependencyGraph;
    }
  else
    {
      G = &(localDependencyGraph.create_subgraph(possibleLoop.begin(),
						 possibleLoop.end())); 
    }
  
  std::vector<vertex_descriptor> component(boost::num_vertices(*G));

  int num = boost::strong_components(*G, &component[0]);
  
  if (num == 1)
    {
      std::transform(possibleLoop.begin(),
		     possibleLoop.end(),
		     possibleLoop.begin(),
		     std::bind2nd(std::plus<vertex_descriptor>(), 1));
      createLocalLoopFormulae(possibleLoop,kb,br);
    }
}

void
LocalLoopFormulaBuilder::checkAllInducedSubGraphs(Loop possibleLoop,
						  vertex_descriptor value,
						  const RulesPtr& kb, 
						  const RulesPtr& br)
{
  if (value == boost::num_vertices(localDependencyGraph))
    {
      possibleLoop.push_back(value-1);
      checkStronglyConnected(possibleLoop,kb,br);
    }
  else
    {
      possibleLoop.push_back(value-1);
      if (possibleLoop.size() > 1)
	{
	  checkStronglyConnected(possibleLoop,kb,br);
	}

      for (vertex_descriptor i = value+1;
	   i <= boost::num_vertices(localDependencyGraph);
	   i++)
	{
	  checkAllInducedSubGraphs(possibleLoop, i,kb,br);
	}
    }
}


void 
LocalLoopFormulaBuilder::buildLambda(const RulesPtr& kb, const RulesPtr& br)
{

  for (vertex_descriptor i = 1; i < boost::num_vertices(localDependencyGraph); i++)
    {
      Loop possibleLoop; // start with empty loop
      checkAllInducedSubGraphs(possibleLoop, i,kb,br);
    }

  boost::graph_traits<Graph>::vertex_iterator vit;
  boost::graph_traits<Graph>::vertex_iterator vend;


  // this part is needed because we consider loops of length 1
  for (boost::tie(vit, vend) = boost::vertices(localDependencyGraph);
       vit != vend; ++vit)
    {
      Loop singleton(1, (*vit+1));
      createLocalLoopFormulae(singleton, kb, br);
    }
}


void
LocalLoopFormulaBuilder::createLocalLoopFormulae(Loop loop, const RulesPtr& kb, const RulesPtr& br) 
{
  Loop::const_iterator lbeg = loop.begin();
  Loop::const_iterator lend = loop.end();
  
  Rules::const_iterator kbeg = kb->begin();
  Rules::const_iterator kend = kb->end();
  Rules::const_iterator bbeg = br->begin();
  Rules::const_iterator bend = br->end();

  std::vector<Rules::const_iterator> esr = externalSupportRules(lbeg, lend, kbeg, kend);
  std::vector<Rules::const_iterator> sr = supportRules(lbeg, lend, bbeg, bend);
  createSupportFormula(lbeg,lend,esr,sr);  
}
