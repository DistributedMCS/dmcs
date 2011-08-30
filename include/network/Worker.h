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
 * @file   Worker.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Aug  29 17:51:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef WORKER_H
#define WORKER_H

#include "network/ConcurrentMessageQueue.h"
#include <boost/thread.hpp>

namespace dmcs {

struct Worker
{
  Worker()
    : sat_thread(0), join_thread(0), 
      request_mq(0), busy(false), 
      k1(0), k2(0)
  { }


  ~Worker()
  {
    if (sat_thread)
      {
	delete sat_thread;
	sat_thread = 0;
      }

    if (join_thread)
      {
	delete join_thread;
	join_thread = 0;
      }

    if (request_mq)
      {
	delete request_mq;
	request_mq = 0;
      }
  }


  boost::thread* sat_thread;
  boost::thread* join_thread;
  ConcurrentMessageQueue* request_mq;

  // status information
  bool busy;
  std::size_t k1;
  std::size_t k2;
};


typedef boost::shared_ptr<Worker> WorkerPtr;
typedef std::vector<WorkerPtr> WorkerVec;
typedef boost::shared_ptr<WorkerVec> WorkerVecPtr;

} // namespace dmcs

#endif // WORKER_H

// Local Variables:
// mode: C++
// End:
