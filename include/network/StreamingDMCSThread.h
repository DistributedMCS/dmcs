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
 * @file   StreamingDMCSThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  12 16:48:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_DMCS_THREAD_H
#define STREAMING_DMCS_THREAD_H

#include "dmcs/StreamingCommandType.h"
#include "network/ConcurrentMessageQueue.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

class StreamingDMCSThread
{
public:
  StreamingDMCSThread(const StreamingCommandTypePtr& scmt_,
		      const ConcurrentMessageQueuePtr& nfh);

  void
  operator()();

private:
  const StreamingCommandTypePtr      scmt;
  ConcurrentMessageQueuePtr          notif_from_handler;
};

typedef boost::shared_ptr<StreamingDMCSThread> StreamingDMCSThreadPtr;

} // namespace dmcs


#endif // STREAMING_DMCS_THREAD_H

// Local Variables:
// mode: C++
// End:
