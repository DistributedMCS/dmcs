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
 * @file   UNSATNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  14 18:42:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef UNSAT_NOTIFICATION_H
#define UNSAT_NOTIFICATION_H

#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

namespace dmcs {

struct UnsatNotification
{
  enum NotificationType
    {
      REQUEST = 0,
      NEXT,
      SHUTDOWN
    };


  UnsatNotification(ConflictVec2p* cs,
		    PartialBeliefState* pa,
		    Decisionlevel* d,
		    std::size_t sid,
		    NotificationType t = REQUEST)
    : all_new_conflicts(cs),
      partial_ass(pa),
      decision(d),
      session_id(sid),
      type(t)
  { 
    assert(cs != 0 || t == SHUTDOWN || t == NEXT);
    assert(t == REQUEST || t == SHUTDOWN || t == NEXT);
  }
  
  /// list of vector of conflicts
  ConflictVec2p* all_new_conflicts;

  /// partial assignment
  PartialBeliefState* partial_ass;

  /// decision level
  Decisionlevel* decision;

  std::size_t session_id;

  /// notification type
  NotificationType type;
};


inline std::ostream&
operator<< (std::ostream& os, const UnsatNotification& un)
{
  return os << "session id: " << un.session_id << std::endl
	    << "conflicts: " << *(un.all_new_conflicts) << std::endl
	    << "partial_ass: " << *(un.partial_ass) << std::endl
	    << "decision: " << *(un.decision);
}

} // namespace dmcs

#endif // UNSAT_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
