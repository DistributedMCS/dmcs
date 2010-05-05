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
#include <boost/shared_ptr.hpp>


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
typedef boost::shared_ptr<NeighborList> NeighborPtr;


} // namespace dmcs

#endif // NEIGHBOR_H

// Local Variables:
// mode: C++
// End:
