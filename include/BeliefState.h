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

#include <iostream>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/tokenizer.hpp>


namespace dmcs {


typedef unsigned long BeliefSet;
typedef std::vector<BeliefSet> BeliefSets;
typedef boost::shared_ptr<BeliefSets> BeliefSetsPtr;


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
  return !(b & 0x1); 
}


struct BeliefState
{
  BeliefSets belief_state;

  BeliefState()
  { }

  BeliefState(std::size_t n)
    : belief_state(n, 0)
  { }

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & belief_state;
  }
};


inline bool
operator< (const BeliefState& bs1, const BeliefState& bs2)
{
  assert (bs2.belief_state.size() == bs1.belief_state.size());
  return std::lexicographical_compare(bs1.belief_state.begin(), bs1.belief_state.end(),
				      bs2.belief_state.begin(), bs2.belief_state.end());
}


inline bool
operator<= (const BeliefState& bs1, const BeliefState& bs2)
{
  assert (bs2.belief_state.size() == bs1.belief_state.size());

  BeliefSets::const_iterator bs1_beg = bs1.belief_state.begin();
  BeliefSets::const_iterator bs1_end = bs1.belief_state.end();
  BeliefSets::const_iterator bs2_beg = bs2.belief_state.begin();
  BeliefSets::const_iterator bs2_end = bs2.belief_state.end();

  std::pair<BeliefSets::const_iterator, BeliefSets::const_iterator> mm =
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

struct BeliefStatePtr
{
  boost::shared_ptr<BeliefState> belief_state_ptr;

   BeliefStatePtr()
     : belief_state_ptr(new BeliefState)
   {  }

   BeliefStatePtr(boost::shared_ptr<BeliefState>& p)
     : belief_state_ptr(p)
   { }

   BeliefStatePtr(BeliefState* p)
     : belief_state_ptr(p)
   { }

   BeliefStatePtr(const BeliefStatePtr& p)
     : belief_state_ptr(p.belief_state_ptr)
   { }

   BeliefStatePtr&
   operator=(const BeliefStatePtr& p)
   {
     if (this->belief_state_ptr != p.belief_state_ptr)
       {
	 this->belief_state_ptr = p.belief_state_ptr;
       }
     return *this;
   }

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & belief_state_ptr;
  }
};


inline bool
operator< (const BeliefStatePtr& p1, const BeliefStatePtr& p2)
{
  return *p1.belief_state_ptr < *p2.belief_state_ptr;
}

inline bool
operator<= (const BeliefStatePtr& p1, const BeliefStatePtr& p2)
{
  return *p1.belief_state_ptr <= *p2.belief_state_ptr;
}

struct BeliefStates
{
  typedef std::set<BeliefStatePtr> BeliefStateSet;

  BeliefStateSet belief_states;

  std::size_t system_size;

  /// do not use default ctor in your code, use ctor with std::size_t!!
  BeliefStates()
    : system_size(0)
  { }

  BeliefStates(std::size_t n)
    : system_size(n)
  { }

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & belief_states;
    ar & system_size;
    assert(system_size > 0); // after the de-serialization, the system_size must be positive
  }
};



struct BeliefStatesPtr
{
  boost::shared_ptr<BeliefStates> belief_states_ptr;

  BeliefStatesPtr()
  { }

  BeliefStatesPtr(boost::shared_ptr<BeliefStates>& p)
    : belief_states_ptr(p)
  { }

  BeliefStatesPtr(BeliefStates* p)
    : belief_states_ptr(p)
  { }

  BeliefStatesPtr(const BeliefStatesPtr& p)
    : belief_states_ptr(p.belief_states_ptr)
  { }

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & belief_states_ptr;
  }
};


inline std::istream& 
operator>>(std::istream& is, BeliefStatePtr& bs)
{
  std::string input;
  std::getline(is, input);

  boost::tokenizer<> s(input);
  for (boost::tokenizer<>::iterator it = s.begin(); it != s.end(); ++it)
    {
      bs.belief_state_ptr->belief_state.push_back(std::atoi(it->c_str()));
    }
  
  return is;
}


inline std::ostream&
operator<< (std::ostream& os, const BeliefSets& bs)
{
  for (BeliefSets::const_iterator it = bs.begin(); it != bs.end(); ++it)
    {
      os << *it << " ";
    }
  
  return os;
}

/*
inline std::ostream&
operator<< (std::ostream& os, const BeliefSets& bs)
{
  for (BeliefSets::const_iterator it = bs.begin(); it != bs.end(); ++it)
    {
      const BeliefSet b = *it;

      if (isEpsilon(b))
	{
	  os << "e";
	}
      else
	{
	  os << "{";

	  std::size_t i = 1; // ignore epsilon bit
	  bool first = true;

	  for (; i < sizeof(b)*8 - 1; ++i)
	    {
	      if (b & (1 << i))
		{
		  if (!first)
		    {
		      os << ", ";
		    }
		  else 
		    {
		      first = false;
		    }

		  os << i;
		}
	    }
	  
	  if (b & (1 << i))
	    {
	      if (!first)
		{
		  os << ", ";
		}

	      os << i;
	    }
	  
	  os << "}";
	
	}

      if (std::distance(it, bs.end()) > 1)
	{
	  os << ", ";
	}
    }

  return os;
}
*/


inline std::ostream&
operator<< (std::ostream& os, const BeliefStatePtr& bs)
{
  return os << bs.belief_state_ptr->belief_state;
}

inline std::ostream&
operator<< (std::ostream& os, const BeliefStatesPtr& bs_ptr)
{
  std::copy(bs_ptr.belief_states_ptr->belief_states.begin(),
	    bs_ptr.belief_states_ptr->belief_states.end(),
	    std::ostream_iterator<BeliefStatePtr>(os, "\n"));

  return os;
}


} // namespace dmcs

#endif // BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
