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

#include <list>

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
    data = (DataType*)malloc(sizeof(DataType)*n);
    for (std::size_t i = 0; i < n; ++i)
      {
	free_slots.push_back(data[i]);
      }
  }



  ~Pool()
  {
    free_slots.clear();
    free(data);
  }



  DataType* malloc()
  {
    boost::mutex::scoped_lock lock(mtx);

    if (free_slots.empty())
      {
	cond.wait(lock);
      }

    DataType* slot = *free_slots.begin();
    free_slots.pop_front();

    return slot;
  }



  void free(DataType* slot)
  {
    boost::mutex::scoped_lock lock(mtx);
    
    free_slots.push_front(slot);

    cond.notify_one();
  }

private:
  std::size_t n;                    // fixed size of the pool
  DataType* data;                   // the array where we store data
  std::list<DataType*> free_slots;

  boost::mutex mtx;
  boost::condition_variable cond;
};

} // namespace dmcs

#endif // POOL_H

// Local Variables:
// mode: C++
// End:
