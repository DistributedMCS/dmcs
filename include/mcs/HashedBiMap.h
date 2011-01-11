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
 * @file   HashedBiMap.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan 11 12:04:20 2011
 * 
 * @brief  Hashed-Bi-Map
 * 
 * 
 */

#ifndef HASHED_BI_MAP_H
#define HASHED_BI_MAP_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/shared_ptr.hpp>


#include <string>
#include <iosfwd>

namespace dmcs {


struct Int2Int
{
  std::size_t first;
  std::size_t second;

  Int2Int(std::size_t f, std::size_t s)
    : first(f), second(s)
  { }
};


inline std::ostream&
operator<< (std::ostream& os, const Int2Int& i2i)
{
  return os << '(' << i2i.first << ' ' << i2i.second << ')';
}



/// tags used to access pairs by first or second element
namespace Tag
{
  struct First {};
  struct Second {};
}


/// a HashedBiMap is a table with 2 columns named First, and Second
typedef boost::multi_index_container<
  Int2Int,
  boost::multi_index::indexed_by<
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<Tag::First>,
      boost::multi_index::member<Int2Int, std::size_t, &Int2Int::first>
      >,
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<Tag::Second>,
      boost::multi_index::member<Int2Int, std::size_t, &Int2Int::second>
      >
    >
  > HashedBiMap;


typedef boost::shared_ptr<HashedBiMap> HashedBiMapPtr;


inline std::ostream&
operator<< (std::ostream& os, const HashedBiMap& hbm)
{
  if (!hbm.empty())
    {
      //      HashedBiMap::const_iterator end = hbm.end();
      //      --end;

      if (hbm.size() > 1)
	{
	  std::copy(hbm.begin(), hbm.end(), std::ostream_iterator<Int2Int>(os, ","));
	}
      //      os << *end;
    }

  return os;
}



inline std::ostream&
operator<< (std::ostream& os, const HashedBiMapPtr& hbm)
{
  return os << *hbm;
}


/// query HashedBiMap by first element
typedef boost::multi_index::index<HashedBiMap, Tag::First>::type HashedBiMapByFirst;

/// query HashedBiMap by second element
typedef boost::multi_index::index<HashedBiMap, Tag::Second>::type HashedBiMapBySecond;

} // namespace dmcs

#endif // HASHED_BI_MAP_H

// Local Variables:
// mode: C++
// End:
