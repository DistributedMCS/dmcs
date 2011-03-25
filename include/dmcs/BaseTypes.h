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
 * @file   BaseTypes.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Mar  21 8:16:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <iterator>
#include <list>
#include <boost/shared_ptr.hpp>

namespace dmcs {

typedef std::list<std::size_t> History;
typedef boost::shared_ptr<History> HistoryPtr;

inline std::ostream&
operator<< (std::ostream& os, const History& hist)
{
  std::copy(hist.begin(), hist.end(), std::ostream_iterator<std::size_t>(os, " "));
  return os;
}

#ifdef DEBUG
  typedef History PathList;
#else
  typedef std::size_t PathList;
#endif

} // namespace dmcs

#endif // BASE_TYPES_H

// Local Variables:
// mode: C++
// End:
