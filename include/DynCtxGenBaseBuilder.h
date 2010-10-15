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
 * @file   DynCtxGenBaseBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jul   6 14:56:00 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DYNAMIC_CONTEXT_GENERATOR_BASE_BUILDER
#define DYNAMIC_CONTEXT_GENERATOR_BASE_BUILDER

#include <list>
#include <vector>
#include <sstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>

#define INC "inc"
#define DEC "dec"

namespace dmcs {

struct Match
{
  Match(std::size_t srcCtx_, std::string srcSym_,
	std::size_t tarCtx_, std::string tarSym_,
	float quality_)
    : srcCtx(srcCtx_), srcSym(srcSym_),
      tarCtx(tarCtx_), tarSym(tarSym_),
      quality(quality_)
  { }

  std::size_t srcCtx;
  std::string srcSym;
  std::size_t tarCtx;
  std::string tarSym;
  float quality;
};


std::ostream&
operator<< (std::ostream& os, const Match& m)
{
  os << "(" << m.srcCtx << " " << m.srcSym << " " << m.tarCtx << " " << m.tarSym << " " << m.quality << ")";

  return os;
}

typedef std::vector<std::size_t> IntVec;
typedef boost::shared_ptr<IntVec> IntVecPtr;

typedef std::vector<std::string> BridgeRuleVec;
typedef boost::shared_ptr<BridgeRuleVec> BridgeRuleVecPtr;
typedef std::vector<BridgeRuleVecPtr> BridgeRuleVecVec;
typedef boost::shared_ptr<BridgeRuleVecVec> BridgeRuleVecVecPtr;
typedef std::vector<Match> MatchVec;

typedef boost::shared_ptr<MatchVec> MatchVecPtr;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
typedef boost::shared_ptr<Graph> GraphPtr;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::edge_descriptor   Edge;


class DynCtxGenBaseBuilder 
{
public:
  DynCtxGenBaseBuilder(MatchVecPtr mt_, GraphPtr initial_topology_, 
		       BridgeRuleVecVecPtr bridge_rules_, 
		       BridgeRuleVecPtr pattern_bridge_rules_,
		       IntVecPtr no_sbridge_atoms_,
		       std::size_t poolsize_)
    : mt(mt_),
      initial_topology(initial_topology_),
      bridge_rules(bridge_rules_),
      pattern_bridge_rules(pattern_bridge_rules_),
      no_sbridge_atoms(no_sbridge_atoms_),
      poolsize(poolsize_)
  { }

  virtual void
  generate_dynamic_system() = 0;

  void
  generate_matches(std::size_t from, std::size_t to)
  {
    std::size_t int_pos_quality = (rand() % 10) + 1;
    std::size_t int_neg_quality = (rand() % 10) + 1;
    float float_pos_quality = 0.1 * int_pos_quality;
    float float_neg_quality = 0.1 * int_neg_quality;
    
    Match m1(from, "pos", to, "s", float_pos_quality);
    Match m2(from, "neg", to, "e", float_neg_quality);
    
    mt->push_back(m1);
    mt->push_back(m2);
    
    // update the topological graph
    Vertex u = boost::vertex(from-1, *initial_topology);
    Vertex v = boost::vertex(to-1, *initial_topology);
    boost::add_edge(u, v, *initial_topology);
  }

  
  void
  generate_bridge_rules(std::size_t id)
  {
    std::stringstream out;
    out << id;
    std::size_t nosba = 0;

    if ((*bridge_rules)[id-1]->size() == 0)
      {
	std::size_t flipping = (rand() % 3);

	nosba += 2;
	std::string s = (*pattern_bridge_rules)[0];
	s = s + "(" + out.str() + ":" + DEC + ").\n";

	(*bridge_rules)[id-1]->push_back(s);

	if (flipping > 0)
	  {
	    nosba += 2;

	    std::string s = (*pattern_bridge_rules)[1];
	    s = s + "(" + out.str() + ":" + INC + ").\n";
	    (*bridge_rules)[id-1]->push_back(s);
	  }

	if (flipping > 1)
	  {
	    nosba += 2;

	    std::string s = (*pattern_bridge_rules)[2];
	    s = s + "\n";
	    (*bridge_rules)[id-1]->push_back(s);	    
	  }

	(*no_sbridge_atoms)[id-1] += nosba;
      }
  }

protected:
  MatchVecPtr mt;
  GraphPtr initial_topology;
  BridgeRuleVecVecPtr bridge_rules;
  BridgeRuleVecPtr pattern_bridge_rules;
  IntVecPtr no_sbridge_atoms;
  std::size_t poolsize;
};


} // namespace dmcs

#endif // DYNAMIC_CONTEXT_GENERATOR_BASE_BUILDER

// Local Variables:
// mode: C++
// End:
