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
 * @file   testCache.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Dec  4 11:02:21 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <iostream>
#include "mcs/CachePosition.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testCache"
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testCachPosition )
{
  CachePosition cp;
  cp.update_cache(1,3);
  cp.update_cache(2,3);
  cp.update_cache(3,3);
  cp.update_cache(4,3);
  cp.update_cache(5,3);
  cp.update_cache(6,3);
  cp.update_cache(7,3);
  cp.update_cache(8,3);
  cp.update_cache(9,3);
  cp.update_cache(10,3);
  cp.update_cache(11,3);
  cp.update_cache(12,3);
  cp.update_cache(13,3);
  cp.update_cache(14,3);
  cp.update_cache(15,3);

  std::cerr << "Update finished" << std::endl;

  static const std::size_t ask[] = {3, 5, 10, 12, 20, 23, 25, 27};
  std::vector<std::size_t> ask_vec(ask, ask + sizeof(ask)/sizeof(ask[0]));

  for (std::vector<std::size_t>::const_iterator it = ask_vec.begin(); it != ask_vec.end(); ++it)
    {
      std::cerr << "Find position for value = " << *it << std::endl;
      std::size_t pos = cp.find_position(*it);
      std::cerr << "value = " << *it << ", pos = " << pos << std::endl;
    }
}


// Local Variables:
// mode: C++
// End:
