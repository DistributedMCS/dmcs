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
 * @file   Pool.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Sep  23 7:50:23 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef POOL_H
#define POOL_H

#include <iostream>
#include <vector>

#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

class Pool
{
public:
  Pool(std::size_t limit, std::size_t ds)
    : n(limit), back_slot(n), datasize(ds)
  {
    data = ::malloc(n * datasize);
    free_slots = (void**)::malloc(n * sizeof(std::size_t));

    // initialize free_slots
    for (std::size_t i = 0; i < n; ++i)
      {
	free_slots[i] =
          reinterpret_cast<void*>(
              reinterpret_cast<uint8_t*>(data) + i * datasize);
      }
  }



  ~Pool()
  {
    back_slot = 0;
    ::free(free_slots);
    ::free(data);
  }



  void* malloc()
  {
    boost::mutex::scoped_lock lock(mtx);

    if (back_slot == 0)
      {
	cond.wait(lock);
      }

    void* slot = free_slots[--back_slot];

    return slot;
  }



  void free(void* slot)
  {
    boost::mutex::scoped_lock lock(mtx);
    
    assert ( slot >= data && slot <= ((n-1) * datasize + data) );
    assert ( 0 <= back_slot && back_slot <= n-1 );

    free_slots[back_slot++] = slot;

    cond.notify_one();
  }

private:
  std::size_t n;         // fixed size of the pool
  std::size_t back_slot;
  std::size_t datasize;

  void*  data;       // the array where we store data
  void** free_slots; // the list of free slots

  boost::mutex mtx;
  boost::condition_variable cond;
};

} // namespace dmcs

#endif // POOL_H

// vim:ts=8:tw=100:
// Local Variables:
// mode: C++
// End:
