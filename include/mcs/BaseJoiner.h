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
  * @file   BaseJoiner.h
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Tue Jan  3 10:18:28 2012
  * 
  * @brief  
  * 
  * 
  */

#ifndef BASE_JOINER_H
#define BASE_JOINER_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "mcs/NewBeliefState.h"
#include "network/NewJoinerDispatcher.h"
#include "mcs/NewNeighbor.h"
#include "network/NewConcurrentMessageDispatcher.h"

namespace dmcs {

class BaseJoiner
{
public:
  BaseJoiner(NewNeighborVecPtr n);

  void
  registerJoinIn(std::size_t offset,
		 NewConcurrentMessageDispatcherPtr md);

protected:
  void
  init();

  void
  cleanup_input(std::size_t noff);

  virtual void
  reset();

protected:
  std::size_t joiner_offset; // offset of the joiner in JOIN_IN_MQ
  NewNeighborVecPtr neighbors;
  NewBeliefStatePackage input_belief_states;
  ReturnedBeliefStateList joined_results;
};

} // namespace dmcs

#endif // BASE_JOINER_H

// Local Variables:
// mode: C++
// End:
