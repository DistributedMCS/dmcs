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
 * @file   QueryPlan.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  19 14:20:21 2010
 * 
 * @brief  
 * 
 * 
 */

#include "QueryPlan.h"
#include "QueryPlanWriter.h"

using namespace dmcs;
using namespace dmcs::generator;

QueryPlan::QueryPlan()
  : query_plan(0),
    gname(boost::get_property(query_plan, boost::graph_name))
{
  setupProperties();
}


void
QueryPlan::setupProperties()
{ 
  name = get(boost::vertex_name, query_plan);
  dp.property(NODE_ID_LB, name);
  
  index = get(boost::vertex_index, query_plan);
  dp.property(INDEX_LB, index);
  
  hostname = get(hostname_t(), query_plan);
  dp.property(HOSTNAME_LB, hostname);
  
  port = get(port_t(), query_plan);
  dp.property(PORT_LB, port);
  
  sigma = get(sigma_t(), query_plan);
  dp.property(SIGMA_LB, sigma);
  
  interface = get(interface_t(), query_plan);
  dp.property(INTERFACE_LB, interface);

  dp.property(GNAME_LB, gname);
}



const BeliefStatePtr&
QueryPlan::getGlobalV()
{
  return gname[&query_plan];
}

void
QueryPlan::putGlobalV(const BeliefStatePtr& V_)
{
  gname[&query_plan] = V_;
}


const Signature&
QueryPlan::getSignature(std::size_t context_id) const
{
  std::map<std::size_t,Vertex>::const_iterator it = vmap.find(context_id-1);
  assert(it != vmap.end());
  Vertex v = boost::vertex(it->second, query_plan);

  return boost::get(sigma_t(), query_plan, v);

#if 0
  std::string s = boost::get(sigma_t(), query_plan, v);

  std::istringstream iss(s);

  Signature sig;
  iss >> sig;

  return sig;
#endif
}

const Signature&
QueryPlan::getSignature1(std::size_t context_id) const
{

  Vertex v = boost::vertex(context_id -1, query_plan);

  return boost::get(sigma_t(), query_plan, v);
}

const std::string&
QueryPlan::getHostname(std::size_t context_id) const
{
  std::map<std::size_t,Vertex>::const_iterator it = vmap.find(context_id-1);
  assert(it != vmap.end());
  Vertex v = boost::vertex(it->second, query_plan);
  return boost::get(hostname_t(), query_plan, v);
}


const std::string&
QueryPlan::getHostname1(std::size_t context_id) const
{
  Vertex v = boost::vertex(context_id-1, query_plan);
  return boost::get(hostname_t(), query_plan, v);
}


const std::string&
QueryPlan::getPort(std::size_t context_id) const
{
  std::map<std::size_t,Vertex>::const_iterator it = vmap.find(context_id-1);
  assert(it != vmap.end());
  Vertex v = boost::vertex(it->second, query_plan);
  return boost::get(port_t(), query_plan, v);
}


const std::string&
QueryPlan::getPort1(std::size_t context_id) const
{
  Vertex v = boost::vertex(context_id-1, query_plan);
  return boost::get(port_t(), query_plan, v);
}



std::size_t
QueryPlan::getSystemSize()
{
  return boost::num_vertices(query_plan);
}


void
QueryPlan::putSignature(std::size_t context_id, const Signature& signature_)
{
#if 0
  std::ostringstream oss;
  oss << signature_;
#endif
  boost::put(sigma, context_id-1, signature_);
}


void
QueryPlan::putHostname(std::size_t context_id, const std::string& hostname_)
{
  boost::put(hostname, context_id-1, hostname_);
}


void
QueryPlan::putPort(std::size_t context_id, const std::string& port_)
{
  boost::put(port, context_id-1, port_);
}


const BeliefStatePtr&
QueryPlan::getInterface1(std::size_t context1, std::size_t context2) const
{
  EdgeIter ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(query_plan);

  for (; ei != ei_end; ++ei)
    {
      std::size_t id1 = index[boost::source(*ei, query_plan)];
      std::size_t id2 = index[boost::target(*ei, query_plan)];
      
      if ((context1 - 1 == id1) && (context2 - 1 == id2))
	{
	  break;
	}
    }

  return boost::get(interface_t(), query_plan, *ei);
}


const BeliefStatePtr&
QueryPlan::getInterface(std::size_t context1, std::size_t context2) const
{
  EdgeIter ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(query_plan);

  for (; ei != ei_end; ++ei)
    {
      //std::size_t id1 = index[boost::source(*ei, query_plan)];
      //std::size_t id2 = index[boost::target(*ei, query_plan)];
      std::size_t id1 = name[boost::source(*ei, query_plan)];
      std::size_t id2 = name[boost::target(*ei, query_plan)];
      
      if ((context1 - 1 == id1) && (context2 - 1 == id2))
	{
	  break;
	}
    }

  return boost::get(interface_t(), query_plan, *ei);
}


void
QueryPlan::putInterface(std::size_t context1, std::size_t context2, const BeliefStatePtr& interface_)
{
  EdgeIter ei, ei_end;
  boost::tie(ei, ei_end) = boost::edges(query_plan);

  for (; ei != ei_end; ++ei)
    {
      //std::size_t id1 = index[boost::source(*ei, query_plan)];
      //std::size_t id2 = index[boost::target(*ei, query_plan)];
      std::size_t id1 = index[boost::source(*ei, query_plan)];
      std::size_t id2 = index[boost::target(*ei, query_plan)];

      //std::cerr << "Putting interface to " << context1 << " " << context2 
      //		<< " (" << id1 << " " << id2 << ")" << std::endl;
      if ((context1 == id1+1) && (context2 == id2+1))
	{
	  boost::put(interface, *ei, interface_);
	  break;
	}
    }
}


void
QueryPlan::add_neighbor(std::size_t context_id1, std::size_t context_id2)
{
  //Vertex u = boost::vertex(vmap[context_id1-1], query_plan);
  //Vertex v = boost::vertex(vmap[context_id2-1], query_plan);
  //std::cerr << "Adding neighbors: " << context_id1 - 1 << " " << context_id2-1 << std::endl;
  Vertex u = boost::vertex(context_id1-1, query_plan);
  Vertex v = boost::vertex(context_id2-1, query_plan);

  boost::add_edge(u, v, query_plan);
}


void
QueryPlan::remove_connection(std::size_t context_id1, std::size_t context_id2)
{
  //Vertex u = boost::vertex(vmap[context_id1-1], query_plan);
  //Vertex v = boost::vertex(vmap[context_id2-1], query_plan);
  Vertex u = boost::vertex(context_id1-1, query_plan);
  Vertex v = boost::vertex(context_id2-1, query_plan);
  //std::cerr << "removing connection" << std::endl;
  //std::cerr << "context_id1 " << context_id1 << ", context_id2 " << context_id2 << std::endl;
  boost::remove_edge(u, v, query_plan);
}


NeighborsPtr
QueryPlan::getNeighbors1(std::size_t context_id)
{
  NeighborsPtr nbs(new Neighbors);
  
  Vertex v = boost::vertex(context_id-1, query_plan);
  
  OutEdgeIter out_i;
  OutEdgeIter out_end;
  
  boost::tie(out_i, out_end) = out_edges(v, query_plan); 
  for (; out_i != out_end; ++out_i) 
    {
      Vertex t = boost::target(*out_i, query_plan);
      std::size_t nid = index[t];
      //std::cerr << "nid: " << nid << std::endl;
      nbs->push_back(nid+1);
    }

  return nbs;
}


NeighborsPtr
QueryPlan::getNeighbors(std::size_t context_id)
{
  //std::cerr << "in get Neighbors with VMAP"<< std::endl;
  NeighborsPtr nbs(new Neighbors);
  
  Vertex v = boost::vertex(vmap[context_id-1], query_plan);
  OutEdgeIter out_i;
  OutEdgeIter out_end;
  
  boost::tie(out_i, out_end) = out_edges(v, query_plan); 
  for (; out_i != out_end; ++out_i) 
    {
      Vertex t = boost::target(*out_i, query_plan);
      std::size_t nid = name[t];
      nbs->push_back(nid+1);
    }

  return nbs;
}


EdgesPtr
QueryPlan::getEdges()
{
  EdgesPtr edges(new Edges);
  EdgeIter ei;
  EdgeIter ei_end;

  boost::tie(ei, ei_end) = boost::edges(query_plan);

  for (; ei != ei_end; ++ei)
    {
      Vertex s = boost::source(*ei, query_plan);
      Vertex t = boost::target(*ei, query_plan);

      std::size_t s_id = index[s];
      std::size_t t_id = index[t];

      EdgeInt p(s_id+1, t_id+1);
      edges->push_back(p);
    }

  return edges;
}

graph_t
QueryPlan::getGraph()
{
  return query_plan;
}


void
QueryPlan::putGraph(graph_t query_plan_)
{
  query_plan = query_plan_;
}

void
QueryPlan::read_graph(std::istream& iss)
{
  boost::read_graphviz(iss, query_plan, dp, NODE_ID_LB);

  VertexIter it;
  VertexIter end;

  for (boost::tie(it, end) = boost::vertices(query_plan); it != end; ++it)
    {
      // here we assume that name[n] is an integer
      ///@todo this should be a bimap...
      vmap.insert(std::make_pair(name[*it], *it));
    }
}

void
QueryPlan::read_graph(const std::string& filename)
{
  std::ifstream f(filename.c_str());
  if (f.is_open()) 
    {
      read_graph(f);
    }
}


void
QueryPlan::write_graph(std::ostream& os)
{
  std::stringstream ss;
  ss << gname[&query_plan];
  std::string s = ss.str();
  boost::write_graphviz(os, query_plan, 
			make_vertex_writer(hostname, port, sigma), 
			make_edge_writer(interface),
			make_graph_writer(s));
}
