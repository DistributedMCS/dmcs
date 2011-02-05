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
	  
#ifdef DEBUG
	  std::cerr << "bb: " << *jt << std::endl;
#endif
	}
      
      // postive body
      const PositiveBridgeBody& pb = getPositiveBody(*it);
      for (PositiveBridgeBody::const_iterator jt = pb.begin();jt != pb.end(); ++jt)
	{
	  assert(jt->second > 0);

	  std::size_t id = jt->second + placement;

	  assert (id);

	  //put placement
	  (result->second.first)->push_back(id);
	  fillKappaPositiveBody(id);
	  
#if defined(DEBUG)
	  std::cerr << "posb: " << jt->second << " " << id << std::endl;
#endif //DEBUG
	}
      
      // negative body
      const NegativeBridgeBody& nb = getNegativeBody(*it);
      for (NegativeBridgeBody::const_iterator jt = nb.begin(); jt != nb.end(); ++jt)
	{
	  assert(jt->second > 0);

	  std::size_t id = jt->second + placement;

	  assert (id);

	  //put placement
	  (result->second.second)->push_back(id);
	  fillKappaNegativeBody(id);
	  
#ifdef DEBUG
	  std::cerr << "negb: " << jt->second << " " << id << std::endl;
#endif
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
						Graph* graph,
						const RulesPtr& kb,
						const RulesPtr& br)
{
  Graph *G = 0;
  
  // this if condition is needed, otherwise a segmentation fault occurs
  if (possibleLoop.size() == boost::num_vertices(*graph))
    {
      G = graph;
    }
  else
    {
      G = &((*graph).create_subgraph(possibleLoop.begin(),
				     possibleLoop.end()));
    }

  assert (G != 0);
  
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
						  Graph* graph,
						  unsigned int index,
						  Loop& component_graph,
						  const RulesPtr& kb,
						  const RulesPtr& br)
{
  if (index == boost::num_vertices(*graph))
    {
      possibleLoop.push_back(component_graph[index-1]);
      checkStronglyConnected(possibleLoop,graph,kb,br);
    }
  else
    {
      possibleLoop.push_back(component_graph[index-1]);
      if (possibleLoop.size() > 1)
	{
	  checkStronglyConnected(possibleLoop,graph,kb,br);
	}

      for (unsigned int i = index+1;
	   i <= boost::num_vertices(*graph);
	   i++)
	{
	  checkAllInducedSubGraphs(possibleLoop,graph,index,component_graph,kb,br);
	}
    }
}

//patched version of the original strategy
//1) first compute the SCC components of the graph
//2) for each subgraph strongly connected explore all the subset of its vertex as before
//3) @todo further optimization is needed (e.g., removing vertex or edges from the actual strong connected component)

void 
LocalLoopFormulaBuilder::buildLambda(const RulesPtr& kb, const RulesPtr& br)
{
  //first compute the connected components of the original graph
  std::vector<vertex_descriptor> component (boost::num_vertices(localDependencyGraph));
  int num = boost::strong_components(localDependencyGraph, &component[0]);
  Loops component_graphs(num);
  for (vertex_descriptor i = 1; i < boost::num_vertices(localDependencyGraph); i++)
    {
      component_graphs[component[i]].push_back(i);
    }
  
  //now for each component explore the subset of nodes
  Graph* graph;
  for(int k=0;k<num;k++)
    {
      graph = &(localDependencyGraph.create_subgraph(component_graphs[k].begin(),
						     component_graphs[k].end()));
      
      
      for (unsigned int i = 1; i < boost::num_vertices(*graph); i++)
	{
	  Loop possibleLoop; // start with empty loop
	  checkAllInducedSubGraphs(possibleLoop, graph,i,component_graphs[k],kb,br);
	}
    }

#if defined(DEBUG)  
  std::cerr<<"FINISHED "<<std::endl;
#endif//DEBUG

  boost::graph_traits<Graph>::vertex_iterator vit;
  boost::graph_traits<Graph>::vertex_iterator vend;


  // this part is needed because we consider loops of length 1
  for (boost::tie(vit, vend) = boost::vertices(localDependencyGraph);
       vit != vend; ++vit)
    {
      Loop singleton(1, (*vit+1));
      createLocalLoopFormulae(singleton, kb, br);

#if defined(DEBUG)
      std::cerr<<"HERE";
#endif//DEBUG
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
  
#if 0
  std::cout<<"CREATING LOOP "<<std::endl;
  for( Loop::const_iterator lbeg = loop.begin();lbeg!=loop.end();lbeg++)
    std::cout<<*lbeg<<std::endl;
  
  std::cout<<"CREATING LOOP END"<<std::endl;
#endif//0
  
  std::vector<Rules::const_iterator> esr = externalSupportRules(lbeg, lend, kbeg, kend);
  std::vector<Rules::const_iterator> sr = supportRules(lbeg, lend, bbeg, bend);
  createSupportFormula(lbeg,lend,esr,sr);
}
