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

#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

template <typename DataType>
class Pool
{
public:
  Pool(int limit)
    : n(limit), free_point(0)
  {
    data = (DataType*)malloc(sizeof(DataType)*n);
    used = (bool*)malloc(sizeof(bool)*n);
    for (std::size_t i = 0; i < n; ++i)
      {
	used[i] = false;
      }
  }



  ~Pool()
  {
    free(used);
    free(data);
  }



  int malloc()
  {
    boost::mutex::scoped_lock lock(mtx);

    if (free_point >= n)
      {
	cond.wait(lock);
      }

    int latest_free_point = free_point;

    while (free_point < n && !used[free_point])
      {
	++free_point;
      }

    used[latest_free_point] = true;

    return latest_free_point;
  }



  DataType* slot(int index)
  {
    assert (index >= 0 && index < n);
    return data[index];
  }



  void free(int index)
  {
    boost::mutex::scoped_lock lock(mtx);

    assert (index >= 0 && index < n && used[index]);
    
    used[index] = false;

    if (index < free_point)
      {
	free_point = index;
      }

    cond.notify_one();
  }

private:
  int n;           // fixed size of the pool
  int free_point;  // the lowest point in the pool that is free
  DataType* data;  // the array where we store data
  bool*     used;  // to mark whether a chunk of memory is already in used

  boost::mutex mtx;
  boost::condition_variable cond;
};

} // namespace dmcs

#endif // POOL_H

// Local Variables:
// mode: C++
// End:
