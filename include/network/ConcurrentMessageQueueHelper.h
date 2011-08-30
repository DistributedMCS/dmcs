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
 * @file   ConcurrentMessageQueueHelper.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  16 14:16:21 2011
 * 
 * @brief  
 * 
 * 
 */


#ifndef CONCURRENT_MESSAGE_QUEUE_HELPER_H
#define CONCURRENT_MESSAGE_QUEUE_HELPER_H

#include "dmcs/Log.h"
#include "network/ConcurrentMessageQueue.h"
#include "network/ConcurrentMessageQueueFactory.h"

namespace dmcs {

inline void*
overwrite_send(ConcurrentMessageQueue* cmq,
	       const void* buf,
	       std::size_t size,
	       unsigned int prio)
{
  void* tmp_buf = 0;
  assert (size == sizeof(buf));

  if (!cmq->try_send(buf, size, prio))
    {
      std::size_t tmp_prio = 0;
      std::size_t recvd = 0;

      if (!cmq->try_receive(&tmp_buf, sizeof(tmp_buf), recvd, tmp_prio))
	{
	  tmp_buf = 0;
	}

      cmq->send(buf, size, prio);
    }

  return tmp_buf;
}



inline MessagingGatewayBC::ModelSession
receive_model(ConcurrentMessageQueue* cmq)
{
  struct MessagingGatewayBC::ModelSession ms = {0, 0, 0};
  std::size_t recvd = 0;
  unsigned int p = 0;
  void* ptr = static_cast<void*>(&ms);
  cmq->receive(ptr, sizeof(ms), recvd, p);

  return ms;
}


inline AskNextNotification*
receive_notification(ConcurrentMessageQueue* cmq)
{
  AskNextNotification* notif = 0;
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&notif);

  cmq->receive(ptr, sizeof(notif), recvd, p);

  assert(sizeof(notif) == recvd);

  return notif;
}


inline StreamingForwardMessage*
receive_incoming_message(ConcurrentMessageQueue* cmq)
{
  StreamingForwardMessage* m = 0;
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&m);

  cmq->receive(ptr, sizeof(m), recvd, p);

  assert(sizeof(m) == recvd);

  return m;
}


} // namespace dmcs


#endif // CONCURRENT_MESSAGE_QUEUE_HELPER_H

// Local Variables:
// mode: C++
// End:

