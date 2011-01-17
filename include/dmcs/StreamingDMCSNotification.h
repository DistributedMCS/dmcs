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
 * @file   StreamingDMCSNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  13 7:10:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_DMCS_NOTIFICATION_H
#define STREAMING_DMCS_NOTIFICATION_H

#include <boost/thread/future.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcs {

struct StreamingDMCSNotification
{
  StreamingDMCSNotification(std::size_t invoker_,
			    std::size_t pack_size_,
			    std::size_t port_)
    : invoker(invoker_), 
      pack_size(pack_size_), 
      port(port_)
  { }
  
  std::size_t invoker;
  std::size_t pack_size;
  std::size_t port;
};

  /*
typedef boost::shared_ptr<StreamingDMCSNotification> StreamingDMCSNotificationPtr;
typedef boost::promise<StreamingDMCSNotificationPtr> StreamingDMCSNotificationPromise;
typedef boost::unique_future<StreamingDMCSNotificationPtr> StreamingDMCSNotificationFuture;
typedef boost::shared_ptr<StreamingDMCSNotificationFuture> StreamingDMCSNotificationFuturePtr;
  */

} // namespace dmcs

#endif // STREAMING_DMCS_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
