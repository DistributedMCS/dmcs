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
 * @file   Conflict.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Jan  4 11:12:18 2011
 * 
 * @brief  
 * 
 * 
 */

#include <boost/circular_buffer.hpp>

#include "mcs/BeliefState.h"

#if !defined(CONFLICT_H)
#define CONFLICT_H

namespace dmcs {

/**
 * @brief a Conflict is just a PartialBeliefState for now
 */
typedef PartialBeliefState Conflict;

///@todo: check whether we can use ConflictPtr here instead of Conflict*
typedef std::vector<Conflict*> ConflictVec;
typedef boost::shared_ptr<ConflictVec> ConflictVecPtr;
typedef std::vector<ConflictVecPtr> ConflictVec2;
typedef std::vector<ConflictVec*> ConflictVec2p;
typedef boost::shared_ptr<ConflictVec2> ConflictVec2Ptr;

// no operator<< is needed for ConflictVec, ConflictVecPtr,
// ConflictVec2, ConflictVec2Ptr as long as Conflict ==
// PartialBeliefState.


typedef boost::circular_buffer<Conflict*> ConflictBuf;
typedef boost::shared_ptr<ConflictBuf> ConflictBufPtr;

typedef std::vector<ConflictBufPtr> ConflictBufVec;
typedef boost::shared_ptr<ConflictBufVec> ConflictBufVecPtr;

typedef std::vector<ConflictBuf::iterator> ConflictBufIterVec;
typedef boost::shared_ptr<ConflictBufIterVec> ConflictBufIterVecPtr;


inline bool
cached(Conflict* c, ConflictBufPtr& storage)
{
  for (ConflictBuf::const_iterator it = storage->begin(); it != storage->end(); ++it)
    {
      if (*c == **it)
	{
	  return true;
	}
    }
  return false;
}


inline std::ostream&
operator<< (std::ostream& os, const ConflictVec2p& cv)
{
  for (ConflictVec2p::const_iterator it = cv.begin(); it != cv.end(); ++it)
    {
      os << **it << std::endl;
    }

  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const ConflictBuf& cb)
{
  for (ConflictBuf::const_iterator it = cb.begin(); it != cb.end(); ++it)
    {
      os << **it << std::endl;
    }
  return os;
}



inline std::ostream&
operator<< (std::ostream& os, const ConflictBufVec& cbv)
{
  for (ConflictBufVec::const_iterator it = cbv.begin(); it != cbv.end(); ++it)
    {
      os << **it << std::endl;
    }
  return os;
}

} // namespace dmcs

#endif // CONFLICT_H

// Local Variables:
// mode: C++
// End:
