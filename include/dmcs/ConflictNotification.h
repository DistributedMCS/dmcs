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

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/shared_ptr.hpp>

#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

namespace dmcs {

struct ConflictNotification
{
  ConflictNotification(Conflict* c,
		       PartialBeliefState* pa)
    : val(0),
      conflict(c),
      partial_ass(pa)
  { }

  ConflictNotification(std::size_t val_,
		       Conflict* conflict_,
		       PartialBeliefState* partial_ass_)
    : val(val_),
      conflict(conflict_),
      partial_ass(partial_ass_)
  { }
  
  std::size_t  val;                // from SAT    --> Router:      val == id of the neighbor to send the assignment
                                   // from Router --> NeighborOut: val is now unused
  Conflict*           conflict;
  PartialBeliefState* partial_ass; // partial assignment
};

} // namespace dmcs

#endif // CONFLICT_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
