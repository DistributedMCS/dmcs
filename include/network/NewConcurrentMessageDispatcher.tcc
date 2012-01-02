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
 * @file   NewConcurrentMessageDispatcher.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  28 21:48:26 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_CONCURRENT_MESSAGE_DISPATCHER_TCC
#define NEW_CONCURRENT_MESSAGE_DISPATCHER_TCC

namespace dmcs {

template<typename MessageType>
bool
NewConcurrentMessageDispatcher::send(MQIDs id, MessageType* mess, int msecs)
{
  assert (id < SEPARATOR);
  ConcurrentMessageQueuePtr cmq = getMQ(id);
  return send(cmq.get(), mess, msecs);
}



template<typename MessageType>
bool
NewConcurrentMessageDispatcher::send(MQIDs type, std::size_t id, 
				     MessageType* mess, int msecs)
{
  assert (SEPARATOR < type && type < END_OF_MQ);
  ConcurrentMessageQueuePtr cmq = getMQ(type, id);
  return send(cmq.get(), mess, msecs);
}



template<typename MessageType>
bool
NewConcurrentMessageDispatcher::send(ConcurrentMessageQueue* cmq,
				     MessageType* mess, int msecs)
{
  assert (cmq);

  bool ret = true;

  if (msecs > 0)
    {
      ret = cmq->timed_send(&mess, sizeof(mess), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      ret = cmq->try_send(mess, sizeof(mess), 0);
    }
  else
    {
      cmq->send(&mess, sizeof(mess), 0);
    }

  return ret;
}



template<typename MessageType>
MessageType*
NewConcurrentMessageDispatcher::receive(MQIDs id, int msecs)
{
  assert (id < SEPARATOR);
  ConcurrentMessageQueuePtr cmq = getMQ(id);
  return receive<MessageType>(cmq.get(), msecs);
}



template<typename MessageType>
MessageType*
NewConcurrentMessageDispatcher::receive(MQIDs type, std::size_t id, int msecs)
{
  assert (SEPARATOR < type && type < END_OF_MQ);
  ConcurrentMessageQueuePtr cmq = getMQ(type, id);
  return receive<MessageType>(cmq.get(), msecs);
}



template<typename MessageType>
MessageType*
NewConcurrentMessageDispatcher::receive(ConcurrentMessageQueue* cmq, int msecs)
{
  assert (cmq);

  MessageType* mess = 0;
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&mess);

  if (msecs > 0)
    {
      if (!cmq->timed_receive(ptr, sizeof(mess), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  mess = 0;
	  return mess;
	}
    }
  else if (msecs < 0)
    {
      if (!cmq->try_receive(ptr, sizeof(mess), recvd, p))
	{
	  msecs = 0;
	  mess = 0;
	  return mess;
	}
    }
  else
    {
      cmq->receive(ptr, sizeof(mess), recvd, p);
    }

  assert(sizeof(mess) == recvd);
  return mess;
}

} // namespace dmcs

#endif // NEW_CONCURRENT_MESSAGE_DISPATCHER_TCC

// Local Variables:
// mode: C++
// End:
