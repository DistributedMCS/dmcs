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
 * @file   NetworkPointers.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Nov 12:04:26 15 2012
 *
 * @brief 
 *
 *
 */

#ifndef __NETWORK_POINTERS_H__
#define __NETWORK_POINTERS_H__

#include <boost/shared_ptr.hpp>

namespace dmcs {

class NewServer;
class NewHandler;
class HandlerWrapper;

typedef boost::shared_ptr<NewServer> NewServerPtr;
typedef boost::shared_ptr<NewHandler> NewHandlerPtr;

} // namespace dmcs

#endif // __NETWORK_POINTERS_H__

// Local Variables:
// mode: C++
// End:
