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

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/tokenizer.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include <bm/bm.h>
#include <bm/bmserial.h>


namespace dmcs {

/// a belief set
typedef bm::bvector<> BeliefSet;

/// a belief state
typedef std::vector<BeliefSet> BeliefState;
typedef boost::shared_ptr<BeliefState> BeliefStatePtr;

/// a list of belief states, use BeliefStateCmp and BeliefStateEq for sort() and unique(), resp.
typedef std::list<BeliefStatePtr> BeliefStateList;
typedef boost::shared_ptr<BeliefStateList> BeliefStateListPtr;

typedef std::vector<BeliefState*> BeliefStateVec;
typedef boost::shared_ptr<BeliefStateVec> BeliefStateVecPtr;
typedef std::vector<BeliefStateVecPtr> BeliefStatePackage;
typedef boost::shared_ptr<BeliefStatePackage> BeliefStatePackagePtr;

typedef std::vector<BeliefStateVec::const_iterator> BeliefStateIteratorVec;
typedef boost::shared_ptr<BeliefStateIteratorVec> BeliefStateIteratorVecPtr;


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



/// @brief read BeliefSet from stream
inline std::istream&
operator>> (std::istream& is, BeliefSet& belief)
{
  std::string s;
  std::getline(is, s);
 
  typedef boost::char_separator<char> separator;
  typedef boost::tokenizer<separator> tokenizer;

  separator sep("{ }");
  tokenizer tok(s, sep);

  for (tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it)
    {
      std::size_t bit = std::atoi(it->c_str());
      belief.set(bit);
    }

  return is;
}


} //namespace dmcs


namespace std {

/** 
 * Output true bits in belief set @a bv on @a os.
 * 
 * @param os 
 * @param bv 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::BeliefSet& bv)
{
  std::size_t count = bv.count();
  if (count == 0)
    {
      os << "[ ]";
      return os;
    }

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
    }
  while (bit);

  return os << "]";
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
operator>> (std::istream& is, dmcs::BeliefStatePtr& bs)
{
  using namespace dmcs;

  std::string s;
  std::getline(is, s);
  BeliefSet empty;     // an partial belief set

  // BeliefState is one of the forms:

  // (1) {belief} {belief} ... {belief}
  //     where a partial belief is a list of unsingned integers representing the true atoms/bits.

  // (2) belief belief ... belief
  //     where each belief is an integer representing a full assignment,
  //     meaning that when a bit is on, the corresponding atom is set to true.
  
  // (2) is for the sake of backward compatiblity.

  if (!bs)
    {
      bs = BeliefStatePtr(new BeliefState);
    }


  // case (2)
  if (s.find("{") == std::string::npos)
    {
      const std::size_t ONE = 1;
      std::size_t atom;
      std::size_t remain;
      std::size_t i;

      boost::tokenizer<> tok(s);

      for (boost::tokenizer<>::const_iterator it = tok.begin(); it != tok.end(); ++it)
	{
	  // if input is invalid, boost::lexical_cast will throw an
	  // error. For now we don't catch it, just let it crash.
	  atom = boost::lexical_cast<std::size_t>(*it);
	  i    = 0;
	  bs->push_back(empty);
	  
	  while (atom > 0)
	    {
	      remain = atom & ONE;  // remain = atom % 2
	      atom >>= 1;           // atom   = atom / 2

	      if (remain)
		{
		  if (i == 0)
		    {
		      setEpsilon(bs->back());
		    }
		  else
		    {
		      setBeliefSet(bs->back(), i);
		    }
		}

	      ++i;
	    }
	}

      return is;
    }


  // case (1)
  typedef boost::char_separator<char> separator;
  typedef boost::tokenizer<separator> tokenizer;

  separator sep(" ", "{}");
  tokenizer tok(s, sep);

  enum State
  {
    START,
    BELIEFSET
  };

  State state = START;
  
  for (tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it)
    {
      const char *t = it->c_str();

      switch (state)
	{
	case START:
	  {
	    assert( *t == '{' ); // we expect an open curly bracket
	    bs->push_back(empty);
	    state = BELIEFSET;
	    break;
	  }
	case BELIEFSET:
	  {
	    if (*t == '}') // are we there yet?
	      {
		state = START;
	      }
	    else
	      {
		std::size_t atom = boost::lexical_cast<std::size_t>(t);
		if (atom == 0)
		  {
		    setEpsilon(bs->back());
		  }
		else 
		  {
		    setBeliefSet(bs->back(), atom);
		  }
	      }
	    break;
	  }
	}
    }
  
  return is;
}



/**
 * Output space-separated belief state @a bs on @a os.
 *
 * @param os
 * @param bs
 *
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::BeliefState& bs)
{
  using namespace dmcs;

  /*std::copy(bs.begin(), bs.end(),
	    std::ostream_iterator<BeliefSet>(os, " ")
	    );*/
  for (BeliefState::const_iterator it = bs.begin(); it != bs.end(); ++it)
    {
      os << *it << " ";
    }
  return os;
}


/** 
 * Output space-separated @a bs on @a os.
 * 
 * @param os 
 * @param bs 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::BeliefStatePtr& bs)
{
  using namespace dmcs;

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
operator<< (std::ostream& os, const dmcs::BeliefStateList& l)
{
  using namespace dmcs;

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


/** 
 * Output newline-separated @a l on @a os.
 * 
 * @param os 
 * @param l 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::BeliefStateListPtr& l)
{
  using namespace dmcs;

  std::copy(l->begin(), l->end(), std::ostream_iterator<BeliefStatePtr>(os, "\n"));

  return os;
}

} // namespace std




// ********************************************************************************
// PartialBeliefState, for streaming DMCS

namespace dmcs {

typedef bm::bvector<> BitMagic;

/// a partial belief set
struct PartialBeliefSet
{
  // for 3-value assignment
  enum TruthVal
    {
      DMCS_FALSE = 0,
      DMCS_TRUE,
      DMCS_UNDEF
    };

  PartialBeliefSet()
  { }

  PartialBeliefSet(std::size_t n)
    : value_bit(n),
      state_bit(n)
  { }

  PartialBeliefSet(const PartialBeliefSet& pb)
  {
    assert (pb.value_bit.size() == pb.state_bit.size());
    value_bit = pb.value_bit;
    state_bit = pb.state_bit;
  }

  inline std::size_t
  size() const
  {
    assert (value_bit.size() == state_bit.size());
    return value_bit.size();
  }

  void
  resize(std::size_t n)
  {
    assert (n > 0);
    value_bit.resize(n);
    state_bit.resize(n);
  }

  BitMagic value_bit;
  BitMagic state_bit;
};


typedef std::vector<PartialBeliefSet> PartialBeliefState;
typedef boost::shared_ptr<PartialBeliefState> PartialBeliefStatePtr;
typedef std::vector<PartialBeliefState*> PartialBeliefStateVec;
typedef boost::shared_ptr<PartialBeliefStateVec> PartialBeliefStateVecPtr;

typedef std::vector<PartialBeliefStateVecPtr> PartialBeliefStatePackage;
typedef boost::shared_ptr<PartialBeliefStatePackage> PartialBeliefStatePackagePtr;

typedef std::vector<PartialBeliefStateVec::const_iterator> PartialBeliefStateIteratorVec;
typedef boost::shared_ptr<PartialBeliefStateIteratorVec> PartialBeliefStateIteratorVecPtr;


inline bool
operator== (const PartialBeliefSet& p, const PartialBeliefSet& b)
{
  return ((p.value_bit == b.value_bit) && (p.state_bit == b.state_bit));
}


inline bool
operator!= (const PartialBeliefSet& p, const PartialBeliefSet& b)
{
  return !(p == b);
}


inline bool
operator< (const PartialBeliefSet& p, const PartialBeliefSet& b)
{
  if (p.state_bit < b.state_bit)
    {
      return true;
    }
  else if ((p.state_bit == b.state_bit ) && (p.value_bit < b.value_bit))
    {
      return true;
    }
  return false;
}



inline PartialBeliefSet::TruthVal
testBeliefSet(PartialBeliefSet& pb, std::size_t pos)
{
  assert (pos > 0 && pos < pb.size());

  if (pb.state_bit.test(pos))
    {
      if (pb.value_bit.test(pos))
	{
	  return PartialBeliefSet::DMCS_TRUE;
	}
      else
	{
	  return PartialBeliefSet::DMCS_FALSE;
	}
    }
  
  return PartialBeliefSet::DMCS_UNDEF;
}



inline void
setBeliefSet(PartialBeliefSet& pb, std::size_t pos, 
	     PartialBeliefSet::TruthVal val = PartialBeliefSet::DMCS_TRUE)
{
  assert (pos > 0 && pos < pb.size());

  if (val == PartialBeliefSet::DMCS_UNDEF)
    {
      pb.state_bit.set_bit(pos, false);
      pb.value_bit.set_bit(pos, false); // just want to have it clean
    }
  else
    {
      pb.state_bit.set_bit(pos);
      if (val == PartialBeliefSet::DMCS_FALSE)
	{
	  pb.value_bit.set_bit(pos, false);
	}
      else 
	{
	  pb.value_bit.set_bit(pos);
	}
    }
}



inline bool
isEpsilon(const PartialBeliefSet& pb)
{
  return !(pb.state_bit.test(0));
}



inline void
setEpsilon(PartialBeliefSet& pb)
{
  pb.state_bit.set(0);
  pb.value_bit.set(0); // just want to have it clean
}


} // namespace dmcs


namespace std
{


/** 
 * Output true/false bits in a partial belief set @a pb on @a os.
 * 
 * @param os 
 * @param pb
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefSet& pb)
{
  using namespace dmcs;

  std::size_t count = pb.state_bit.count();
  if (count == 0)
    {
      os << "[ ]";
      return os;
    }

  os << "[";
  std::size_t bit = pb.state_bit.get_first();
  do
    {
      if (!pb.value_bit.test(bit))
	{
	  os << "-";
	}
      else
	{
	  os << " ";
	}
      os << bit;

      bit = pb.state_bit.get_next(bit);
      
      if (bit)
	{
	  os << ", ";
	}
    }
  while (bit);

  return os << "]";  
}

/**
 * Read a partial belief state from @a is and store it in @a bs.
 *
 * @param is
 * @param bs
 *
 * @return is
 */
inline std::istream& 
operator>> (std::istream& is, dmcs::PartialBeliefState& pbs)
{
  using namespace dmcs;

  std::string s;
  std::getline(is, s);
  PartialBeliefSet empty; // an empty partial belief set

  // A PartialBeliefState is one of the forms:

  // (1) {partial_belief} {partial_belief} ... {partial_belief}
  //     where a partial belief is a list of integers representing a 
  //     partial assingment, where positive (resp. negative) integers represent 

  // (2) belief belief ... belief
  //     where each belief is an integer representing a full assignment,
  //     meaning that when a bit is on, the corresponding atom is set to true.
  
  // (2) is for the sake of backward compatiblity.


  // case (2)
  if (s.find("{") == std::string::npos)
    {
      const std::size_t ONE = 1;
      std::size_t atom;
      std::size_t remain;
      std::size_t i;

      boost::tokenizer<> tok(s);

      for (boost::tokenizer<>::const_iterator it = tok.begin(); it != tok.end(); ++it)
	{
	  // if input is invalid, boost::lexical_cast will throw an
	  // error. For now we don't catch it, just let it crash.
	  atom = boost::lexical_cast<std::size_t>(*it);
	  i    = 0;
	  pbs.push_back(empty);
	  
	  while (atom > 0)
	    {
	      remain = atom & ONE;  // remain = atom % 2
	      atom >>= 1;           // atom   = atom / 2

	      if (remain)
		{
		  if (i == 0)
		    {
		      setEpsilon(pbs.back());
		    }
		  else
		    {
		      setBeliefSet(pbs.back(), i);
		    }
		}

	      ++i;
	    }
	}

      return is;
    }


  // case (1)
  typedef boost::char_separator<char> separator;
  typedef boost::tokenizer<separator> tokenizer;

  separator sep(" ", "{}");
  tokenizer tok(s, sep);

  enum State
  {
    START,
    BELIEFSET
  };

  State state = START;
  
  for (tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it)
    {
      const char *t = it->c_str();

      switch (state)
	{
	case START:
	  {
	    assert( *t == '{' ); // we expect an open curly bracket
	    pbs.push_back(empty);
	    state = BELIEFSET;
	    break;
	  }
	case BELIEFSET:
	  {
	    if (*t == '}') // are we there yet?
	      {
		state = START;
	      }
	    else
	      {
		int atom = boost::lexical_cast<int>(t);
		if (atom == 0)
		  {
		    setEpsilon(pbs.back());
		  }
		else if (atom > 0)
		  {
		    setBeliefSet(pbs.back(), atom);
		  }
		else
		  {
		    setBeliefSet(pbs.back(), -atom, PartialBeliefSet::DMCS_FALSE);
		  }
	      }
	    break;
	  }
	}
    }
  
  return is;
}


/** 
 * Output space-separated @a pbs on @a os.
 * 
 * @param os 
 * @param pbs 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefState& pbs)
{
  using namespace dmcs;

  std::copy(pbs.begin(), pbs.end(), std::ostream_iterator<PartialBeliefSet>(os, " "));

  /*
  for (PartialBeliefState::const_iterator it = pbs.begin(); it != pbs.end(); ++it)
    {
      os << *it << " ";
    }
  */

  return os;
}



/** 
 * Output space-separated @a pbs on @a os.
 * 
 * @param os 
 * @param pbs 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStatePtr& pbs)
{
  os << *pbs;

  return os;
}



/** 
 * Output newline-separated @a pbsv on @a os.
 * 
 * @param os 
 * @param pbsv 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStateVec& pbsv)
{
  using namespace dmcs;
  
  for (PartialBeliefStateVec::const_iterator it = pbsv.begin(); it != pbsv.end(); ++it)
    {
      os << **it << "\n";
    }

  return os;
}



/** 
 * Output newline-separated @a pbsv on @a os.
 * 
 * @param os 
 * @param pbsv 
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStateVecPtr& pbsv)
{
  os << *pbsv;

  return os;
}



/** 
 * Output double-newline-separated @a pbsp on @a os.
 * 
 * @param os 
 * @param pbsp
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStatePackage& pbsp)
{
  using namespace dmcs;

  std::copy(pbsp.begin(), pbsp.end(), std::ostream_iterator<PartialBeliefStateVecPtr>(os, "\n"));

  return os;
}


/** 
 * Output double-newline-separated @a pbsp on @a os.
 * 
 * @param os 
 * @param pbsp
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStatePackagePtr& pbsp)
{
  os << *pbsp;

  return os;
}



/** 
 * Output newline-separated @a pbsi on @a os.
 * 
 * @param os 
 * @param pbsi
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStateIteratorVec& pbsi)
{
  using namespace dmcs;
  
  std::vector<PartialBeliefStateVec::const_iterator>::const_iterator it = pbsi.begin();
  for (; it != pbsi.end(); ++it)
    {
      os << **it << "\n";
    }

  return os;
}



/** 
 * Output newline-separated @a pbsi on @a os.
 * 
 * @param os 
 * @param pbsi
 * 
 * @return os
 */
inline std::ostream&
operator<< (std::ostream& os, const dmcs::PartialBeliefStateIteratorVecPtr& pbsi)
{
  os << *pbsi;

  return os;
}


} // namespace std


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

  unsigned char buf[st.max_serialize_mem];
  unsigned char* b = buf;

  const std::size_t len = bvs.serialize(belief, buf, st.max_serialize_mem);

  ar << len;

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


// ***************************************************************************************
// PartialBeliefState, for streaming DMCS

/**
 * Serialization Method Save.
 * @param Archive ar
 * @param PartialBeliefSet pb
 * @param int version
 *
 */
template<class Archive>
inline void save(Archive & ar, const dmcs::PartialBeliefSet& pb, unsigned int version)
{
  ///@todo TK: hack a new archive thingie, we copy things twice in here

  ///@todo MD: code duplication. Create save/load methods for BitMagic and reuse?

  bm::serializer<dmcs::BitMagic> bm_serializer;
  
  //belief.optimize();

  dmcs::BitMagic::statistics st_value;
  dmcs::BitMagic::statistics st_state;

  pb.value_bit.calc_stat(&st_value);
  pb.state_bit.calc_stat(&st_state);

  unsigned char buf_value[st_value.max_serialize_mem];
  unsigned char buf_state[st_state.max_serialize_mem];

  unsigned char* b_value = buf_value;
  unsigned char* b_state = buf_state;

  const std::size_t len_value = bm_serializer.serialize(pb.value_bit, buf_value, st_value.max_serialize_mem);
  const std::size_t len_state = bm_serializer.serialize(pb.state_bit, buf_state, st_state.max_serialize_mem);

  const std::size_t bit_size = pb.size();

  ar << bit_size;

  ar << len_value;

  for (std::size_t i = 0; i < len_value; ++i)
    {
      ar << *b_value++;
    }

  ar << len_state;

  for (std::size_t i = 0; i < len_state; ++i)
    {
      ar << *b_state++;
    }
  
  ar << version;
}



/**
 * Serialization Method Load.
 * @param Archive ar
 * @param PartialBeliefSet pb
 * @param int version
 *
 */
template<class Archive>
inline void load(Archive & ar, dmcs::PartialBeliefSet& pb, unsigned int version)
{
  std::size_t bit_size  = 0;
  std::size_t len_value = 0;
  std::size_t len_state = 0;

  ar >> bit_size;
  assert (bit_size > 0);

  if (pb.size() != bit_size)
    {
      pb.resize(bit_size);
      pb.resize(bit_size);
    }

  ar >> len_value;
  assert(len_value > 0);

  ///@todo TK: hack a new archive thingie, we copy things twice in here

  unsigned char buf_value[len_value];
  unsigned char* b_value = buf_value;

  for (std::size_t i = 0; i < len_value; i++)
    {
      ar >> *b_value++;
    }

  bm::deserialize(pb.value_bit, buf_value);


  ar >> len_state;
  assert(len_state > 0);

  unsigned char buf_state[len_state];
  unsigned char* b_state = buf_state;

  for (std::size_t i = 0; i < len_state; i++)
    {
      ar >> *b_state++;
    }
  bm::deserialize(pb.state_bit, buf_state);

  ar >> version;
}



/**
 * Serialization Method Serialize.
 * @param Archive ar
 * @param BeliefSet belief
 * @param int file_version
 *
 */
template<class Archive>
inline void serialize(Archive& ar, dmcs::PartialBeliefSet& pb, const unsigned int file_version)
{
  split_free(ar, pb, file_version);
}


} // namespace serialization

} // namespace boost


#endif // BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
