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
 * @file   QueryPlanWriter.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  19 14:02:00 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef QUERY_PLAN_WRITER_H
#define QUERY_PLAN_WRITER_H

#include <iostream>
#include "BeliefState.h"
#include "QueryPlan.h"


namespace dmcs { 

class QueryPlan;

  namespace generator {

class GraphWriter
{
public:
  GraphWriter(const std::string& n)
    : name(n)
  { }

  //template<class Graph>
  void
  operator()(std::ostream& out) const
  {
    out << "graph [name = \"" << name << "\"] " << std::endl;
  }

private:
  const std::string& name;
};


GraphWriter
make_graph_writer(const std::string& name)
{
  return GraphWriter(name);
}




class VertexWriter 
{
public:
  VertexWriter(VertexHostnameProperty hostname_, VertexPortProperty port_, VertexSigmaProperty sigma_)
    : hostname(hostname_),
      port(port_),
      sigma(sigma_)
  { }
  
  //template <class Vertex>
  void operator()(std::ostream& out, const Vertex& v) const {
    out << " [" 
	<< HOSTNAME_LB << "=\"" << hostname[v] << "\", "
	<< PORT_LB     << "=\"" << port[v]     << "\", "
	<< SIGMA_LB    << "=\"" << sigma[v]    
	<< "\"]";
  }
private:
  VertexHostnameProperty hostname;
  VertexPortProperty port;
  VertexSigmaProperty sigma;
};



VertexWriter
make_vertex_writer(VertexHostnameProperty hostname_, 
		   VertexPortProperty port_, 
		   VertexSigmaProperty sigma_)
{
  return VertexWriter(hostname_, port_, sigma_);
}


class EdgeWriter {
public:
  EdgeWriter(EdgeInterfaceProperty interface_) 
    : interface(interface_)
  { }
  
  void operator()(std::ostream& out, const Edge& e) const {
    out << "[" << INTERFACE_LB << "=\"" << interface[e] << "\"]";
  }
private:
  EdgeInterfaceProperty interface;
};


EdgeWriter
make_edge_writer(EdgeInterfaceProperty interface_)
{
  return EdgeWriter(interface_);
}

  } // namespace generator
} // namespace dmcs

#endif // QUERY_PLAN_WRITER_H

// Local Variables:
// mode: C++
// End:
