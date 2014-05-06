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
 * @file   CachePosition.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Dec  4 10:39:01 2012
 * 
 * @brief  
 */

#include <assert.h>
#include <iostream>
#include "mcs/CachePosition.h"

namespace dmcs {

void
CachePosition::update_cache(std::size_t index,
			    std::size_t k)
{
  if (index <= mark.size()) return;

  assert (index == mark.size()+1);

  if (index == 1)
    {
      mark.push_back(k);
    }
  else
    {
      k += mark.back();
      mark.push_back(k);
    }
}

std::size_t
CachePosition::find_position(const std::size_t k)
{
  if (mark.empty())
    {
      return 0;
    }

  // as mark is incremental, we can do a binary search here
  std::size_t beg = 0;
  std::size_t end = mark.size();
  std::size_t mid;

  while (beg+1 < end)
    {
      mid = (beg + end)/2;
      if (mark[mid] < k)
	{
	  beg = mid;
	}
      else if (mark[mid] > k)
	{
	  end = mid;
	}
      else if (mark[mid] == k)
	{
	  return mid;
	}
    }
  
  // actually beg but outside we count from 1
  if (mark[beg] == k)
    {
      return beg;
    }

  return beg+1;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
