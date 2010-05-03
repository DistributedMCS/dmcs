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

#include "OptMessage.h"
#include "PrimitiveMessage.h"
#include "Theory.h"
#include "connection.hpp"

#include <boost/shared_ptr.hpp>

#if !defined(_SESSION_H)
#define _SESSION_H

namespace dmcs {

template <typename MessageType>
struct Session
{
  MessageType mess;
  connection_ptr conn;

  Session(connection_ptr c) : conn(c) { }
};


typedef boost::shared_ptr<Session<PrimitiveMessage> > SessionPrimitivePtr;
typedef boost::shared_ptr<Session<OptMessage> > SessionOptPtr;

} // namespace dmcs

#endif // _SESSION_H

// Local Variables:
// mode: C++
// End:
