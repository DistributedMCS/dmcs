/* DMC -- Distributed Nonmonotonic Multi-Context Systems.
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
 * @file   NewNeighborOut.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Mar  2 9:23:26 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_NEIGHBOR_OUT_H
#define NEW_NEIGHBOR_OUT_H

#include "network/NewConcurrentMessageDispatcher.h"
#include "network/connection.hpp"

namespace dmcs {

class NewNeighborOut
{
public:
  NewNeighborOut();
  ~NewNeighborOut();

  void
  operator()(connection_ptr conn,
	     NewConcurrentMessageDispatcherPtr md,
	     std::size_t neighbor_offset);
};

typedef boost::shared_ptr<NewNeighborOut> NewNeighborOutPtr;

} // namespace dmcs

#endif // NEW_NEIGHBOR_OUT_H

// Local Variables:
// mode: C++
// End:
