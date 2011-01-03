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
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include "bm/bm.h"
#include "bm/bmserial.h"


namespace dmcs {

/// a belief set
typedef bm::bvector<> BeliefSet;
  //typedef uint64_t BeliefSet;

/// a belief state
typedef std::vector<BeliefSet> BeliefState;
typedef boost::shared_ptr<BeliefState> BeliefStatePtr;

/// a list of belief states, use BeliefStateCmp and BeliefStateEq for sort() and unique(), resp.
typedef std::list<BeliefStatePtr> BeliefStateList;
typedef boost::shared_ptr<BeliefStateList> BeliefStateListPtr;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;



/** 
 * @param b 
 * @param pos
 * 
 * @return true if the pos'th bit in b is 1
 */
inline bool
testBeliefSet(const BeliefSet& b, std::size_t pos)
{
  assert (pos > 0 && pos < b.size());
  return b.get_bit(pos);
}


/** 
 * @param b 
 * @param pos
 * @param val
 * 
 * @return set the pos'th bit in b to val
 */
inline void
setBeliefSet(BeliefSet& b, std::size_t pos, bool val = true)
{
  assert (pos > 0 && pos < b.size());
  b.set(pos, val);
}


/** 
 * @return BeliefSet with every bit set to 1
 */
inline BeliefSet
maxBeliefSet()
{
  BeliefSet b;
  b.set();
  return b;
}




/** 
 * @param b 
 * 
 * @return true if b is epsilon, false otw.
 */
inline bool
isEpsilon(const BeliefSet& b)
{
  // we need the first bit true in b for efficient subset checking in
  // non-epsilon BeliefSet's
  return !(b[0]); 
}


/** 
 * @param b 
 * 
 * @return an enabled BeliefSet with epsilon-bit set to 1
 */
inline void
setEpsilon(BeliefSet& b)
{
  // we need the first bit true in b for efficient subset checking in
  // non-epsilon BeliefSet's
  b.set(0);
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





inline std::istream&
operator>> (std::istream& is, BeliefSet& belief)
{
  std::string s;
  std::getline(is, s);
 
  boost::char_separator<char> sep("{ }");
  tokenizer tok(s, sep);
  for (tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it)
    {
      std::size_t bit = std::atoi(it->c_str());
      belief.set(bit);
    }

  return is;
}


inline std::ostream&
operator<< (std::ostream& os, const BeliefSet& bv)
{
  os << "[";
  std::size_t bit = bv.get_first();
  do
    {
      os << bit;
      bit = bv.get_next(bit);
      if (bit)
	{
	  os << ", ";
	}
      else
	{
	  break;
	}
    }
  while (1);
  os << "]";

  return os;
}


/**
 * Read a belief state from is and store it in bs.
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

  std::string s;
  std::getline(is, s);


  // A BeliefState is of the form {belief} {belief} ... {belief}
  // where a belief is a list of integers indicating the on bits
  boost::char_separator<char> sep(" ", "{}");
  tokenizer tok(s, sep);

  std::size_t state = 0;

  for (tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it)
    {
      switch (state)
	{
	case 0:
	  {
	    assert( std::string(it->c_str()).compare("{") == 0);
	    BeliefSet belief;
	    bs->push_back(belief);
	    state = 1;
	    break;
	  }
	case 1:
	  {
	    if (std::string(it->c_str()).compare("}") == 0)
	      {
		state = 0;
	      }
	    else
	      {
		std::size_t bit = std::atoi(it->c_str());
		bs->back().set(bit);
	      }
	    break;
	  }
	}
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
operator<< (std::ostream& os, const BeliefState& bs)
{
  /*std::copy(bs.begin(), bs.end(),
	    std::ostream_iterator<BeliefSet>(os, " ")
	    );*/
  for (BeliefState::const_iterator it = bs.begin(); it != bs.end(); ++it)
    {
      os << *it << " ";
    }
  return os;
}

inline std::ostream&
operator<< (std::ostream& os, const BeliefStatePtr& bs)
{
  /*std::copy(bs->begin(), bs->end(),
	    std::ostream_iterator<BeliefSet>(os, " ")
	    );*/
  for (BeliefState::const_iterator it = bs->begin(); it != bs->end(); ++it)
    {
      os << *it << " ";
    }
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
  //typedef boost::iterator_adaptor<BeliefState,BeliefStateList::const_iterator> my_iterator; 
  //my_iterator first(l.begin());
  //my_iterator last(l.end());
  //std::copy(first, last, std::ostream_iterator<BeliefState>(os, "\n"));
  for (BeliefStateList::const_iterator it = l.begin(); it != l.end(); ++it)
    {
      os << **it << std::endl;
    }
  return os;
}

inline std::ostream&
operator<< (std::ostream& os, const BeliefStateListPtr& l)
{

  std::copy(l->begin(), l->end(), std::ostream_iterator<BeliefStatePtr>(os, "\n"));

  return os;
}


} // namespace dmcs


namespace boost {

namespace serialization {

///Non intrusive implementation of serialization methods for BeliefSet.
///The main serialization method is implemented outside the BeliefSet class
///and splitted in the save/load functions
///See http://www.boost.org/doc/libs/1_43_0/libs/serialization/doc/index.html

/**
 * Serialization Method Save.
 * @param Archive ar
 * @param BeliefSet belief
 * @param int version
 *
 */
template<class Archive>
inline void save(Archive & ar, const dmcs::BeliefSet& belief, unsigned int version)
{
  ///@todo TK: hack a new archive thingie, we copy things twice in here

  bm::serializer<dmcs::BeliefSet> bvs;
  
  //belief.optimize();

  dmcs::BeliefSet::statistics st;
  belief.calc_stat(&st);

#ifdef DEBUG
  std::cerr << "Serializing " << belief << std::endl;
  std::cerr << "Bits count:" << belief.count() << std::endl;
  std::cerr << "Bit blocks:" << st.bit_blocks << std::endl;  
  std::cerr << "GAP blocks:" << st.gap_blocks << std::endl;  
  std::cerr << "Memory used:"<< st.memory_used << std::endl;  
  std::cerr << "Max.serialize mem.:" << st.max_serialize_mem << std::endl;
#endif

  unsigned char buf[st.max_serialize_mem];
  unsigned char* b = buf;

  const std::size_t len = bvs.serialize(belief, buf, st.max_serialize_mem);

  ar << len;

#ifdef DEBUG
  std::cerr << "Serialized size:" << len << std::endl;
#endif

  for (std::size_t i = 0; i < len; ++i)
    {
      ar << *b++;
    }
  
  ar << version;
}



/**
 * Serialization Method Load.
 * @param Archive ar
 * @param BeliefSet belief
 * @param int version
 *
 */
template<class Archive>
inline void load(Archive & ar, dmcs::BeliefSet & belief, unsigned int version)
{
  std::size_t len = 0;

  ar >> len;

  assert(len > 0);

  ///@todo TK: hack a new archive thingie, we copy things twice in here

  unsigned char buf[len];
  unsigned char* b = buf;

  for (std::size_t i = 0; i < len; i++)
    {
      ar >> *b++;
    }

  ar >> version;

  bm::deserialize(belief, buf);
}



/**
 * Serialization Method Serialize.
 * @param Archive ar
 * @param BeliefSet belief
 * @param int file_version
 *
 */
template<class Archive>
inline void serialize(Archive& ar, dmcs::BeliefSet& belief, const unsigned int file_version)
{
  split_free(ar, belief, file_version);
}


} // namespace serialization

} // namespace boost


#endif // BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
