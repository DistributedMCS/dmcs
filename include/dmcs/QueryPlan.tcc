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
 * @file   QueryPlan.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  6 9:36:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef QUERY_PLAN_TCC
#define QUERY_PLAN_TCC


namespace dmcs {

template<typename Prop, typename Ret>
void
getProp(QueryPlanPtr query_plan, std::size_t context_id, Ret& ret)
{
  VertexMapPtr vmap = query_plan->vmap;
  VertexMap::const_iterator it = vmap->find(context_id-1);
  assert(it != vmap->end());
  Vertex v = boost::vertex(it->second, query_plan->graph);
  
  ret = boost::get(Prop(), query_plan->graph, v);
}


template<typename Prop, typename Ret>
void
getProp(QueryPlanPtr query_plan, std::size_t context1, std::size_t context2, Ret& ret)
{
  EdgeIter ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(query_plan->graph);

  bool got_edge = false;

  for (; ei != ei_end; ++ei)
    {
      std::size_t id1 = query_plan->name[boost::source(*ei, query_plan->graph)];
      std::size_t id2 = query_plan->name[boost::target(*ei, query_plan->graph)];
      
      if ((context1 - 1 == id1) && (context2 - 1 == id2))
	{
	  got_edge = true;
	  break;
	}
    }

  assert (got_edge);

  return boost::get(Prop(), query_plan->graph, *ei);
}



template<typename Holder, typename Prop>
void
putProp(std::size_t context_id, Holder& holder, const Prop& prop)
{
  boost::put(holder, context_id-1, prop);
}



template<typename Holder, typename Prop>
void
putProp(QueryPlanPtr query_plan, std::size_t context_id1, 
	std::size_t context_id2, Holder& holder, const Prop& prop)
{
  //@todo: remove iteration to find the edge
  EdgeIter ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(query_plan->graph);

  bool got_edge = false;

  for (; ei != ei_end; ++ei)
    {
      //std::size_t id1 = index[boost::source(*ei, query_plan)];
      //std::size_t id2 = index[boost::target(*ei, query_plan)];
      std::size_t id1 = query_plan->index[boost::source(*ei, query_plan->graph)];
      std::size_t id2 = query_plan->index[boost::target(*ei, query_plan->graph)];
      
      if ((context_id1 - 1 == id1) && (context_id2 - 1 == id2))
	{
	  got_edge = true;
	  break;
	}
    }

  assert (got_edge);

  boost::put(holder, *ei, prop);
}

} // namespace dmcs

#endif // QUERY_PLAN_TCC

// Local Variables:
// mode: C++
// End:
