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
 * @file   BaseStreamer.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  13 21:42:21 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef BASE_STREAMER_H
#define BASE_STREAMER_H

#include "network/ConcurrentMessageQueueFactory.h"
#include "network/connection.hpp"
#include <boost/asio.hpp>

namespace dmcs {

/**
 * @brief
 */
class BaseStreamer
{
public:
  BaseStreamer(boost::asio::io_service& io_service_)
    : conn(new connection(io_service_))
  { }

protected:
  virtual void 
  stream(const boost::system::error_code& error) = 0;

protected:
  connection_ptr     conn;
};

} // namespace dmcs

#endif // BASE_STREAMER_H

// Local Variables:
// mode: C++
// End:
