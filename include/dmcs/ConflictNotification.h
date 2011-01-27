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
 * @file   ConflictNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  14 18:42:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef CONFLICT_NOTIFICATION_H
#define CONFLICT_NOTIFICATION_H

#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

namespace dmcs {

struct ConflictNotification
{
  enum NotificationType
    {
      REQUEST = 0,
      SHUTDOWN
    };


  ConflictNotification(ConflictVec* cs,
		       PartialBeliefState* pa,
		       std::size_t v = 0,
		       NotificationType t = REQUEST)
    : val(v),
      conflicts(cs),
      partial_ass(pa),
      type(t)
  { 
    assert(cs != 0 || t == SHUTDOWN);
    assert(t == REQUEST || t == SHUTDOWN);
  }
  
  // from SAT    --> Router:      val == id of the neighbor to send the assignment
  // from Router --> NeighborOut: val is now unused
  /// neighbor id
  std::size_t val;

  /// list of conflicts
  ConflictVec* conflicts;

  /// partial assignment
  PartialBeliefState* partial_ass;

  /// notification type
  NotificationType type;
};

} // namespace dmcs

#endif // CONFLICT_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
