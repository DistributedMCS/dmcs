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
 * @file   InputBeliefState.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 16:32:24 2012
 * 
 * @brief  Type of information sent from JoinerDispatcher to Joiner (Context).
 *         JoinerDispatcher reads ReturnedBeliefState from NeighborIn, and then
 *         dispatches the answer based on query_id. It finally attaches neighbor_offset
 *         to the answer (which forms an InputBeliefState) to send to the corresponding Context.
 * 
 * 
 */

#ifndef INPUT_BELIEF_STATE_H
#define INPUT_BELIEF_STATE_H

#include <list>
#include <boost/shared_ptr.hpp>

#include "mcs/ReturnedBeliefState.h"
#include "mcs/Printhelpers.h"

namespace dmcs {

struct InputBeliefState : private ostream_printable<InputBeliefState>
{
  InputBeliefState(const ReturnedBeliefState& v, const std::size_t no)
    : value(v), neighbor_offset(no)
  { }

  std::ostream&
  print(std::ostream& os) const
  {
    return os << value << ", noffset = " << neighbor_offset;
  }

  ReturnedBeliefState value;
  std::size_t neighbor_offset;
};
 
} // namespace dmcs

#endif // INPUT_BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
