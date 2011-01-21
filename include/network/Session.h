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
 * @file   Session.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Nov  8 21:21:02 2009
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/Theory.h"

#include "network/connection.hpp"

#include <boost/shared_ptr.hpp>

#if !defined(_SESSION_H)
#define _SESSION_H

namespace dmcs {

  /// holds the data of a session
template <typename MessageType>
struct Session
{
  connection_ptr conn; /// the connection of the session

  MessageType mess;    /// data payload

  Session(const connection_ptr& c)
    : conn(c)
  { }
};


} // namespace dmcs

#endif // _SESSION_H

// Local Variables:
// mode: C++
// End:
