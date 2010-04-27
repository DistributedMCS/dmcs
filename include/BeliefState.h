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
 * @file   BeliefState.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Wed Nov  4 11:15:29 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef BELIEF_STATE_H
#define BELIEF_STATE_H

#include <iosfwd>
#include <limits>
#include <list>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/tokenizer.hpp>


namespace dmcs {

/// a belief set
typedef uint64_t BeliefSet;

/// a belief state
typedef std::vector<BeliefSet> BeliefState;
typedef boost::shared_ptr<BeliefState> BeliefStatePtr;

/// a list of belief states, use BeliefStateCmp and BeliefStateEq for sort() and unique(), resp.
typedef std::list<BeliefStatePtr> BeliefStateList;
typedef boost::shared_ptr<BeliefStateList> BeliefStateListPtr;


/** 
 * @param b 
 * @param pos
 * 
 * @return true if the pos'th bit in b is 1
 */
inline bool
testBeliefSet(BeliefSet b, std::size_t pos)
{
  assert (pos > 0 && pos < sizeof(b)*8);
  return b & ((BeliefSet)1 << pos);
}


/** 
 * @param b 
 * @param pos
 * @param val
 * 
 * @return set the pos'th bit in b to val
 */
inline BeliefSet
setBeliefSet(BeliefSet b, std::size_t pos, bool val = true)
{
  assert (pos > 0 && pos < sizeof(b)*8);
  return val ? ( b | ((BeliefSet)1 << pos) ) : ( b & ~((BeliefSet)1 << pos) );
}


/** 
 * @return BeliefSet with every bit set to 1
 */
inline BeliefSet
maxBeliefSet()
{
  return std::numeric_limits<BeliefSet>::max();
}




/// the first bit is used to enable a BeliefSet
#define EPSILONMASK ((BeliefSet)1 << 0)

/** 
 * @param b 
 * 
 * @return true if b is epsilon, false otw.
 */
inline bool
isEpsilon(BeliefSet b)
{
  // we need the first bit true in b for efficient subset checking in
  // non-epsilon BeliefSet's
  return !(b & EPSILONMASK); 
}


/** 
 * @param b 
 * 
 * @return an enabled BeliefSet with epsilon-bit set to 1
 */
inline BeliefSet
setEpsilon(BeliefSet b)
{
  // we need the first bit true in b for efficient subset checking in
  // non-epsilon BeliefSet's
  return b | EPSILONMASK;
}



/// @brief for sorting BeliefStateLists
struct BeliefStateCmp
{
  bool
  operator() (const BeliefStatePtr& bs1, const BeliefStatePtr& bs2)
  {
    assert (bs1->size() == bs2->size());
    return std::lexicographical_compare(bs1->begin(), bs1->end(),
					bs2->begin(), bs2->end());
  }
};


/// @brief for setting BeliefStateLists unique
struct BeliefStateEq
{
  bool
  operator() (const BeliefStatePtr& bs1, const BeliefStatePtr& bs2)
  {
    assert (bs1->size() == bs2->size());
    return std::equal(bs1->begin(), bs1->end(), bs2->begin());
  }
};


/// @brief less-than-equal for belief states
inline bool
operator<= (const BeliefStatePtr& bs1, const BeliefStatePtr& bs2)
{
  assert (bs2->size() == bs1->size());

  BeliefState::const_iterator bs1_beg = bs1->begin();
  BeliefState::const_iterator bs1_end = bs1->end();
  BeliefState::const_iterator bs2_beg = bs2->begin();
  BeliefState::const_iterator bs2_end = bs2->end();

  std::pair<BeliefState::const_iterator, BeliefState::const_iterator> mm =
    std::mismatch (bs1_beg, bs1_end, bs2_beg);

  if (mm.first == bs1_end && mm.second == bs2_end) // equal
    {
      return true;
    }
  else // mismatch in between, return <
    {
      return *mm.first < *mm.second;
    }
}


/**
 * Read a space-separated belief state from is and store it in bs.
 *
 * @param is
 * @param bs
 *
 * @return is
 */
inline std::istream& 
operator>> (std::istream& is, BeliefStatePtr& bs)
{
  // bs may be NULL, set it up
  if (!bs)
    {
      BeliefStatePtr tmp(new BeliefState);
      bs = tmp;
    }

  std::string input;
  std::getline(is, input);

  boost::tokenizer<> s(input);
  for (boost::tokenizer<>::iterator it = s.begin(); it != s.end(); ++it)
    {
      ///@todo TK: this should be fixed
      bs->push_back(std::atoi(it->c_str()));
    }
  
  return is;
}


/**
 * Output a space-separated belief state bs.
 *
 * @param os
 * @param bs
 *
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const BeliefStatePtr& bs)
{
  std::copy(bs->begin(), bs->end(),
	    std::ostream_iterator<BeliefSet>(os, " ")
	    );
  return os;
}


/**
 * Output a newline-separated list of belief states.
 *
 * @param os
 * @param l
 *
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const BeliefStateList& l)
{
  std::copy(l.begin(), l.end(),
	    std::ostream_iterator<BeliefStatePtr>(os, "\n")
	    );
  return os;
}


} // namespace dmcs

#endif // BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
