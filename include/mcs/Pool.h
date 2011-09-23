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

#include <vector>

#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

template <typename DataType>
class Pool
{
public:
  Pool(std::size_t limit)
    : n(limit)
  {
    data = malloc(n * sizeof(DataType));
    free_slots = malloc(n * sizeof(DataType*));
    back_slot = (DataType*) free_slots;

    // initialize free_slots
    for (std::size_t i = 0; i < n; ++i)
      {
	*(back_slot++) = data + i * sizeof(DataType);
      }
  }



  ~Pool()
  {
    back_slot = 0;
    free(free_slots);
    free(data);
  }



  DataType* malloc()
  {
    boost::mutex::scoped_lock lock(mtx);

    if (back_slot < free_slots)
      {
	cond.wait(lock);
      }

    DataType* slot = *(back_slot--);

    return slot;
  }


  void free(DataType* slot)
  {
    boost::mutex::scoped_lock lock(mtx);
    
    assert ( slot >= data && slot <= ((n-1) * sizeof(DataType) + data) );
    assert ( back_slot <= (n-1) * sizeof(DataType*) + free_slots );

    *(++back_slot) = slot;

    cond.notify_one();
  }

private:
  std::size_t n;         // fixed size of the pool
  DataType data[];       // the array where we store data
  DataType** free_slots; // the list of free slots
  DataType* back_slot;   // point to the last available free slot

  boost::mutex mtx;
  boost::condition_variable cond;
};

} // namespace dmcs

#endif // POOL_H

// Local Variables:
// mode: C++
// End:
