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
  * @file   NewNeighbor.h
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Wed Dec  21 17:10:29 2011
  * 
  * @brief  
  * 
  * 
  */

#ifndef NEW_NEIGHBOR_H
#define NEW_NEIGHBOR_H

#include <vector>
#include <boost/shared_ptr.hpp>

namespace dmcs {

struct NewNeighbor 
{
  std::size_t neighbor_id;
  std::string hostname;
  std::string port;
  BeliefTable output_interface;

  NewNeighbor(std::size_t nid, 
	      const std::string& hn,
	      const std::string& p)
    : neighbor_id(nid),
      hostname(hn),
      port(p)
  { }
};

inline std::ostream&
operator<< (std::ostream& os, const NewNeighbor& nb)
{
  
  os << "(" << nb.neighbor_id << "@" << nb.hostname << ":" << nb.port << ")";
  return os;
}

typedef boost::shared_ptr<NewNeighbor> NewNeighborPtr;
typedef std::vector<NewNeighborPtr> NewNeighborVec;

} // namespace dmcs

#endif // NEW_NEIGHBOR_H

// Local Variables:
// mode: C++
// End:
