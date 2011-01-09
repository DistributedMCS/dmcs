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
 * @file   ConcurrentMessageQueue.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Jan  9 06:57:57 2011
 * 
 * @brief  Inter-thread syncronisation using a queue.
 * 
 * 
 */


#ifndef _CONCURRENT_MESSAGE_QUEUE_H
#define _CONCURRENT_MESSAGE_QUEUE_H

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <queue>
#include <cstdio>
#include <cstring>

namespace dmcs {


  /** 
   * Message queue for inter-thread communication. Modelled after
   * boost::interprocess::message_queue.
   */
  class ConcurrentMessageQueue
  {
  private:

    // data elementes of the queue
    struct Block
    {
      void *m; // the message
      std::size_t s; // length of message
    };

    std::queue<Block> q;   /// holds data of message queue
    const std::size_t n;   /// capacity of message queue
    std::size_t enq; /// enqueuing counter
    std::size_t deq; /// dequeuing counter

    // a mutex lock and the associated condition variable
    mutable boost::mutex mtx;
    boost::condition_variable cnd;


    inline void
    waitOnCapacity(boost::mutex::scoped_lock& lock)
    {
      while (n == q.size()) // maximum capacity reached
	{
	  ++enq;
	  cnd.wait(lock);
	  --enq;
	}
      
      if (deq > 0) // is some consumer waiting?
	{
	  cnd.notify_one(); // notify one consuming thread
	}
    }


    inline void
    waitOnEmpty(boost::mutex::scoped_lock& lock)
    {
      while (q.empty()) // minimum capacity reached
	{
	  ++deq;
	  cnd.wait(lock);
	  --deq;
	}

      if (enq > 0) // is some producer waiting?
	{
	  cnd.notify_one(); // notify one producing thread
	}
    }


  public:

    /// default ctor, capacity is one
    ConcurrentMessageQueue()
      : n(1), enq(0), deq(0)
    { }


    /** 
     * Initialize with capacity, if @a capacity is 0, we force it to be 1.
     * 
     * @param capacity the capacity of this message queue
     */
    ConcurrentMessageQueue(std::size_t capacity)
      : n(capacity > 0 ? capacity : 1), enq(0), deq(0)
    { }


    virtual
    ~ConcurrentMessageQueue()
    {
      while (!q.empty())
	{
	  Block& b = q.front();

	  free(b.m);
	  b.m = NULL;

	  q.pop();
	}
    }

    bool
    empty () const
    {
      boost::mutex::scoped_lock lock(mtx);
      return q.empty();
    }


    bool
    size () const
    {
      return n;
    }


    void
    send (const void* buf, std::size_t size, unsigned int /* prio */)
    {
      boost::mutex::scoped_lock lock(mtx);
      waitOnCapacity(lock);

      void *m = malloc(size);

      if (m == NULL)
	{
	  ::perror("malloc");
	}
      
      std::memcpy(m, buf, size);

      Block b = { m, size };

      q.push(b);
    }


    void
    receive (void *buf, std::size_t size, std::size_t& recvd, unsigned int& /* prio */)
    {
      boost::mutex::scoped_lock lock(mtx);
      waitOnEmpty(lock);

      Block& b = q.front();

      assert (b.s <= size);

      std::memcpy(buf, b.m, b.s);
      recvd = b.s;

      free(b.m);
      b.m = NULL;
      b.s = 0;

      q.pop();
    }

  };


  typedef boost::shared_ptr<ConcurrentMessageQueue> ConcurrentMessageQueuePtr;
  
} // namespace dmcs


#endif // _CONCURRENT_MESSAGE_QUEUE_H

// Local Variables:
// mode: C++
// End:

