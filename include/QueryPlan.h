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
 * @file   QueryPlan.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  19 13:57:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef QUERY_PLAN_H
#define QUERY_PLAN_H


#include "BeliefState.h"
#include "Signature.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

namespace dmcs {


#define NODE_ID_LB   "node_id"
#define INDEX_LB     "index"
#define HOSTNAME_LB  "hostname"
#define PORT_LB      "port"
#define SIGMA_LB     "sigma"
#define INTERFACE_LB "interface"
#define GNAME_LB     "name"



struct sigma_t
{
  typedef boost::vertex_property_tag kind;
};

struct hostname_t
{
  typedef boost::vertex_property_tag kind;
};

struct port_t
{
  typedef boost::vertex_property_tag kind;
};

struct interface_t
{
  typedef boost::edge_property_tag kind;
};


typedef boost::property < boost::vertex_name_t, std::size_t, 
        boost::property < boost::vertex_index_t, std::size_t,
        boost::property < hostname_t, std::string,
        boost::property < port_t, std::string,
	boost::property < sigma_t, Signature > > > > > vertex_p;

typedef boost::property< interface_t, BeliefStatePtr > edge_p;

typedef boost::property< boost::graph_name_t, BeliefStatePtr > graph_p;

typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS, vertex_p, edge_p, graph_p > graph_t;

typedef boost::graph_traits<graph_t>::vertex_descriptor Vertex;
typedef boost::graph_traits<graph_t>::vertex_iterator   VertexIter;
typedef boost::graph_traits<graph_t>::edge_descriptor   Edge;
typedef boost::graph_traits<graph_t>::edge_iterator     EdgeIter;
typedef boost::graph_traits<graph_t>::out_edge_iterator OutEdgeIter;

typedef boost::property_map<graph_t, boost::vertex_name_t>::type  VertexNameProperty;
typedef boost::property_map<graph_t, boost::vertex_index_t>::type VertexIndexProperty;
typedef boost::property_map<graph_t, hostname_t>::type            VertexHostnameProperty;
typedef boost::property_map<graph_t, port_t>::type                VertexPortProperty;
typedef boost::property_map<graph_t, sigma_t>::type               VertexSigmaProperty;
typedef boost::property_map<graph_t, interface_t>::type           EdgeInterfaceProperty;
typedef boost::ref_property_map<graph_t*, BeliefStatePtr>         GraphNameProperty;

/// Neighbors is a list of context ids
typedef std::vector<std::size_t> NeighborList;
typedef boost::shared_ptr<NeighborList> NeighborListPtr;

typedef std::pair<std::size_t, std::size_t> EdgeInt;
typedef std::vector<EdgeInt> Edges;
typedef boost::shared_ptr<Edges> EdgesPtr;



  /// @todo maybe rename to a more appropriate name, this is not the QueryPlan
class QueryPlan
{
private:

  void
  setupProperties();


public:
  QueryPlan();

  //QueryPlan(std::size_t n);

  const BeliefStatePtr&
  getGlobalV();

  void
  putGlobalV(const BeliefStatePtr& V_);

  const Signature&
  getSignature(std::size_t context_id) const;

  const Signature&
  getSignature1(std::size_t context_id) const;

  const std::string&
  getHostname1(std::size_t context_id) const;

  const std::string&
  getPort1(std::size_t context_id) const;

  const std::string&
  getHostname(std::size_t context_id) const;

  const std::string&
  getPort(std::size_t context_id) const;

  std::size_t
  getSystemSize();

  void
  putSignature(std::size_t context_id, const Signature& signature_);

  void
  putHostname(std::size_t context_id, const std::string& hostname_);

  void
  putPort(std::size_t context_id, const std::string& port_);

  const BeliefStatePtr&
  getInterface1(std::size_t context1, std::size_t context2) const;

  const BeliefStatePtr&
  getInterface(std::size_t context1, std::size_t context2) const;

  void
  putInterface(std::size_t context1, std::size_t context2, const BeliefStatePtr& interface_);
 
  void
  add_neighbor(std::size_t context1, std::size_t context2);

  void
  remove_connection(std::size_t context1, std::size_t context2);

  NeighborListPtr
  getNeighbors1(std::size_t context_id);

  NeighborListPtr
  getNeighbors(std::size_t context_id);

  EdgesPtr
  getEdges();

  graph_t
  getGraph();

  void
  putGraph(graph_t query_plan_);

  void
  read_graph(std::istream& iss);

  void
  read_graph(const std::string& filename);

  void
  write_graph(std::ostream& os);

private:
  graph_t query_plan;
  boost::dynamic_properties dp;
  VertexNameProperty name;
  VertexIndexProperty index;
  VertexHostnameProperty hostname;
  VertexPortProperty port;
  VertexSigmaProperty sigma;
  EdgeInterfaceProperty interface;
  GraphNameProperty gname;
  std::map<std::size_t, Vertex> vmap;
};

typedef boost::shared_ptr<QueryPlan> QueryPlanPtr;

} // namespace dmcs

#endif // QUERY_PLAN_H

// Local Variables:
// mode: C++
// End:
