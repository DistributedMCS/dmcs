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
 * @file   JoinIn.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 21:35:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef JOIN_IN_H
#define JOIN_IN_H

#include "mcs/Printhelpers.h"

namespace dmcs {

struct NewJoinIn : private ostream_printable<NewJoinIn>
{
  NewJoinIn(std::size_t n_off, std::size_t no_bs)
    : neighbor_offset(n_off),
      no_belief_state(no_bs)
  { }
  
  std::size_t neighbor_offset;
  std::size_t no_belief_state;

  std::ostream&
  print(std::ostream& os) const
  {
    return os << "(" << neighbor_offset << ", " << no_belief_state << ")" << std::endl;
  }
};

} // namespace dmcs

#endif // JOIN_IN_H

// Local Variables:
// mode: C++
// End:
