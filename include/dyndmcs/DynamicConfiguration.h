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
 * @file   DynamicConfiguration.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Apr 27  09:21:59 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DYNAMIC_CONFIGURATION_H
#define DYNAMIC_CONFIGURATION_H

#include <boost/shared_ptr.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>

#include "dyndmcs/ConfigMessage.h"
#include "dyndmcs/Match.h"
#include "dmcs/Message.h"
#include "dmcs/Neighbor.h"
#include "mcs/Rule.h"
#include "mcs/Signature.h"

namespace dmcs {

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor TopoVertex;
typedef boost::graph_traits<Graph>::edge_descriptor   TopoEdge;
typedef boost::graph_traits<Graph>::out_edge_iterator TopoInEdgeIterator;
typedef boost::graph_traits<Graph>::out_edge_iterator TopoOutEdgeIterator;
typedef boost::graph_traits<Graph>::vertex_iterator   TopoVertexIter;
typedef boost::graph_traits<Graph>::edge_iterator     TopoEdgeIter;

typedef boost::shared_ptr<Graph> GraphPtr;
typedef std::list<GraphPtr> GraphList;
typedef boost::shared_ptr<GraphList> GraphListPtr;

class DynamicConfiguration
{
public:

#ifdef DMCS_STATS_INFO
#else
  typedef ContextSubstitutionList    dynmcs_value_type;
  typedef ContextSubstitutionListPtr dynmcs_return_type;
#endif

  DynamicConfiguration(std::size_t ctx_id_, BridgeRulesPtr bridge_rules_, 
		       const NeighborListPtr context_info_, 
		       const MatchTablePtr mt_,
		       CountVecPtr sba_count_,
		       std::size_t limit_answers_,
		       std::size_t limit_bind_rules_,
		       std::size_t heuristics_,
		       const std::string& prefix_);

  ContextSubstitutionListPtr
  lconfig(ConfigMessage& mess,
	  std::size_t dfs_level = 0);

  BridgeRulesPtr
  getBridgeRules()
  {
    return bridge_rules;
  }

  std::size_t
  getSignal()
  {
    return signal;
  }

  void
  setSignal(std::size_t signal_)
  {
    signal = signal_;
  }

  ContextSubstitutionListPtr
  lconfig(const std::size_t C_root, 
	  BridgeRules::const_iterator r_beg,
	  BridgeRules::const_iterator r_end, 
	  ContextSubstitutionPtr ctx_subtitution_sofar,
	  std::size_t dfs_level = 0);

  ContextSubstitutionListPtr
  invoke_neighbors(const std::size_t C_root,
		   ContextVecIteratorList::const_iterator c_beg,
		   ContextVecIteratorList::const_iterator c_end,
		   ContextSubstitutionPtr ctx_substitution_sofar,
		   std::size_t dfs_level);

  ContextSubstitutionListPtr
  bind_rule(PositiveBridgeBody::const_iterator pb_beg, 
	    PositiveBridgeBody::const_iterator pb_end,
	    NegativeBridgeBody::const_iterator nb_beg,
	    NegativeBridgeBody::const_iterator nb_end,
	    ContextSubstitutionPtr ctx_subtitution_sofar,
	    std::size_t dfs_level);

private:
  GraphListPtr
  compute_topology(ContextSubstitutionListPtr ctx_subs, std::size_t dfs_level);

  GraphPtr
  compute_topology(ContextSubstitutionPtr ctx_sub);

  std::size_t
  resulted_system_size(GraphPtr g);

private:
  std::size_t ctx_id;
  BridgeRulesPtr bridge_rules;
  NeighborListPtr context_info; // all contexts in the pool 
  MatchTablePtr mt;
  CountVecPtr sba_count;
  std::size_t limit_answers;
  std::size_t limit_bind_rules;
  std::size_t heuristics;
  std::size_t no_answers;
  std::size_t no_bind_rules;
  bool stop;
  bool stop_bind_rules;
  const std::string& prefix; 
  std::size_t signal;
};

typedef boost::shared_ptr<DynamicConfiguration> DynamicConfigurationPtr;

} // namespace dmcs

#endif // DYNAMIC_CONFIGURATION_H

// Local Variables:
// mode: C++
// End:
