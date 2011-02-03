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
 * @file   Neighbor.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Jan  3 11:37:27 2010
 * 
 * @brief  
 * 
 * 
 */


#if !defined(NEIGHBOR_H)
#define NEIGHBOR_H

#include <list>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>


namespace dmcs {

/// a lightweight "context" used for easy connection setup
struct Neighbor
{
  std::size_t neighbor_id;
  std::string hostname;
  std::string port;

  Neighbor()
  { }

  Neighbor(std::size_t neighbor_id_,
	   const std::string& hostname_, 
	   const std::string& port_)
    : neighbor_id(neighbor_id_),
      hostname(hostname_), 
      port(port_)
  { }
};


typedef boost::shared_ptr<Neighbor> NeighborPtr;
typedef std::list<NeighborPtr> NeighborList;
typedef boost::shared_ptr<NeighborList> NeighborListPtr;
typedef std::vector<NeighborListPtr> NeighborListVec;
typedef boost::shared_ptr<NeighborListVec> NeighborListVecPtr;


struct compareNeighbors
{
  std::size_t an_id;

  compareNeighbors(std::size_t an_id_)
    : an_id(an_id_)
  { }

  bool
  operator() (const NeighborPtr& nb)
  {
    return (nb->neighbor_id == an_id);
  }
};


inline std::ostream&
operator<< (std::ostream& os, const Neighbor& nb)
{

  os << "(" << nb.neighbor_id << "@" << nb.hostname << ":" << nb.port << ")";
  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const NeighborList& neighbors)
{
  if (!neighbors.empty())
    {
      const NeighborList::const_iterator end = --neighbors.end();
      for (NeighborList::const_iterator it = neighbors.begin(); it != end; ++it)
	{
	  NeighborPtr nb = *it;
	  os << "(" << nb->neighbor_id << "@" << nb->hostname << ":" << nb->port << "), ";
	}
      NeighborPtr nb = *end;
      os << "(" << nb->neighbor_id << "@" << nb->hostname << ":" << nb->port << ")";
    }

  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const NeighborListVec& meta_neighbors)
{
  os << "{\n";

  if (!meta_neighbors.empty())
    {
      for (NeighborListVec::const_iterator it = meta_neighbors.begin(); it != meta_neighbors.end(); ++it)
	{
	  os << **it << std::endl;
	}
    }

  os << "}\n";
  return os;
}

inline std::istream&
operator>> (std::istream& is, Neighbor& nb)
{
  std::string s;
  std::getline(is, s);

  boost::tokenizer<> context_details(s);
  boost::tokenizer<>::const_iterator it = context_details.begin();
 
  if (it == context_details.end())
    {
      throw boost::escaped_list_error("Got no context id!");
    }

  std::size_t context_id = std::atoi(it->c_str());
  ++it;

  if (it == context_details.end())
    {
      throw boost::escaped_list_error("Got no hostname!");
    }

  std::string ctx_hostname = *it;
  ++it;

  if (it == context_details.end())
    {
      throw boost::escaped_list_error("Got no port!");
    }

  std::string ctx_port = *it;

  nb.neighbor_id = context_id;
  nb.hostname = ctx_hostname;
  nb.port = ctx_port;
  return is;
}

} // namespace dmcs

#endif // NEIGHBOR_H

// Local Variables:
// mode: C++
// End:
