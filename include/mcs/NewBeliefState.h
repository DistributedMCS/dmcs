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
 * @file   NewBeliefState.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  14 12:22:36 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_BELIEF_STATE_H
#define NEW_BELIEF_STATE_H

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
#include <boost/circular_buffer.hpp>

#include <bm/bm.h>
#include <bm/bmserial.h>

#include "mcs/Printhelpers.h"

namespace dmcs {

typedef bm::bvector<> BitMagic;

struct NewBeliefState : private ostream_printable<NewBeliefState>
{
  // for 3-value assignment
  enum TruthVal
    {
      DMCS_FALSE = 0,
      DMCS_TRUE,
      DMCS_UNDEF
    };

  NewBeliefState(std::size_t n);
  NewBeliefState(const NewBeliefState& bs);

  NewBeliefState&
  operator= (const NewBeliefState& bs);

  inline bool
  operator== (const NewBeliefState& bs2) const;

  inline bool
  operator!= (const NewBeliefState& bs2) const;

  inline bool
  operator< (const NewBeliefState& bs2) const;

  inline NewBeliefState&
  operator| (const NewBeliefState& bs2);

  inline std::size_t
  size() const;

  inline void
  resize(std::size_t n);

  inline TruthVal
  test(std::size_t pos) const;

  inline void
  set(std::size_t pos,
      TruthVal val = NewBeliefState::DMCS_TRUE);

  inline bool
  isEpsilon(std::size_t ctx_id,
	    const std::vector<std::size_t>& starting_offset) const;

  inline void
  setEpsilon(std::size_t ctx_id,
	     const std::vector<std::size_t>& starting_offset);

  inline void
  clear();

  friend inline void
  and_bm(NewBeliefState& bs, const BitMagic& mask);

  std::ostream&
  print(std::ostream& os) const;

  BitMagic status_bit;
  BitMagic value_bit;
};


// inline functions ************************************************

inline bool
NewBeliefState::operator== (const NewBeliefState& bs2) const
{
  return status_bit == bs2.status_bit && value_bit == bs2.value_bit;
}



inline bool
NewBeliefState::operator!= (const NewBeliefState& bs2) const
{
  return value_bit != bs2.value_bit || status_bit != bs2.status_bit;
}



inline bool
NewBeliefState::operator< (const NewBeliefState& bs2) const
{
  if (status_bit < bs2.status_bit)
    {
      return true;
    }
  else if (status_bit == bs2.status_bit && value_bit < bs2.value_bit)
    {
      return true;
    }
  return false;
}



inline NewBeliefState&
NewBeliefState::operator| (const NewBeliefState& bs2)
{
  status_bit |= bs2.status_bit;
  value_bit  |= bs2.value_bit;

  return *this;
}



inline std::size_t
NewBeliefState::size() const
{
  assert (status_bit.size() == value_bit.size());
  return value_bit.size();
}



inline void
NewBeliefState::resize(std::size_t n)
{
  assert (n > 0);
  status_bit.resize(n);
  value_bit.resize(n);
}



inline NewBeliefState::TruthVal
NewBeliefState::test(std::size_t pos) const
{
  assert (pos > 0 && pos < bs.size());

  if (status_bit.test(pos))
    {
      if (value_bit.test(pos))
	{
	  return NewBeliefState::DMCS_TRUE;
	}
      else
	{
	  return NewBeliefState::DMCS_FALSE;
	}
    }
  
  return NewBeliefState::DMCS_UNDEF;  
}



inline void
NewBeliefState::set(std::size_t pos,
		    NewBeliefState::TruthVal val)
{
  assert (pos > 0 && pos < size());
  
  switch (val)
    {
    case NewBeliefState::DMCS_UNDEF:
      {
	status_bit.set_bit(pos, false);
	value_bit.set_bit(pos, false);
	break;
      }
    case NewBeliefState::DMCS_TRUE:
      {
	status_bit.set_bit(pos);
	value_bit.set_bit(pos);
	break;
      }
    case NewBeliefState::DMCS_FALSE:
      {
	status_bit.set_bit(pos);
	value_bit.set_bit(pos, false);
	break;
      }
    }
}


inline bool
NewBeliefState::isEpsilon(std::size_t ctx_id,
			  const std::vector<std::size_t>& starting_offset) const
{
  return !(status_bit.test(starting_offset[ctx_id]));
}



inline void
NewBeliefState::setEpsilon(std::size_t ctx_id,
			   const std::vector<std::size_t>& starting_offset)
{
  status_bit.set(starting_offset[ctx_id]);
  value_bit.set(starting_offset[ctx_id]); // just want to have it clean
}



inline void
NewBeliefState::clear()
{
  status_bit.clear();
  value_bit.clear();
}


inline void
and_bm(NewBeliefState& bs, const BitMagic& mask)
{
  assert (bs.size() == mask.size());
  bs.status_bit &= mask;
  bs.value_bit &= mask;
}


// prototypes for BeliefStates combination
bool
combine(NewBeliefState& s,
	const NewBeliefState& t,
	const std::vector<std::size_t>& starting_offsets,
	const std::vector<BitMagic*>& masks);

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
 * @param NewBeliefState bs
 * @param int version
 *
 */
template<class Archive>
inline void save(Archive & ar, const dmcs::NewBeliefState& bs, unsigned int version)
{
  ///@todo TK: hack a new archive thingie, we copy things twice in here

  bm::serializer<dmcs::BitMagic> bvs;
  
  //bvs.optimize();

  dmcs::BitMagic::statistics status_st;
  dmcs::BitMagic::statistics value_st;
  bs.status_bit.calc_stat(&status_st);
  bs.value_bit.calc_stat(&value_st);

  unsigned char status_buf[status_st.max_serialize_mem];
  unsigned char value_buf[value_st.max_serialize_mem];

  unsigned char* status_b = status_buf;
  unsigned char* value_b = value_buf;

  const std::size_t status_len = bvs.serialize(bs.status_bit, status_buf, status_st.max_serialize_mem);
  const std::size_t value_len = bvs.serialize(bs.value_bit, value_buf, value_st.max_serialize_mem);

  const std::size_t bit_size = bs.size();

  ar << bit_size;

  ar << status_len;

  for (std::size_t i = 0; i < status_len; ++i)
    {
      ar << *status_b++;
    }
  
  ar << value_len;

  for (std::size_t i = 0; i < value_len; ++i)
    {
      ar << *value_b++;
    }

  ar << version;
}



/**
 * Serialization Method Load.
 * @param Archive ar
 * @param NewBeliefState bs
 * @param int version
 *
 */
template<class Archive>
inline void load(Archive & ar, dmcs::NewBeliefState & bs, unsigned int version)
{
  ///@todo TK: hack a new archive thingie, we copy things twice in here

  std::size_t bit_size = 0;
  std::size_t status_len = 0;
  std::size_t value_len = 0;

  ar >> bit_size;
  assert ( bit_size > 0);

  if (bs.size() != bit_size)
    {
      bs.resize(bit_size);
    }

  ar >> status_len;
  assert(status_len > 0);

  unsigned char status_buf[status_len];
  unsigned char* status_b = status_buf;

  for (std::size_t i = 0; i < status_len; i++)
    {
      ar >> *status_b++;
    }
  bm::deserialize(bs.status_bit, status_buf);


  ar >> value_len;
  assert(value_len > 0);

  unsigned char value_buf[value_len];
  unsigned char* value_b = value_buf;

  for (std::size_t i = 0; i < value_len; i++)
    {
      ar >> *value_b++;
    }

  bm::deserialize(bs.value_bit, value_buf);

  ar >> version;
}



/**
 * Serialization Method Serialize.
 * @param Archive ar
 * @param NewBeliefState bs
 * @param int file_version
 *
 */
template<class Archive>
inline void serialize(Archive& ar, dmcs::NewBeliefState& bs, const unsigned int file_version)
{
  split_free(ar, bs, file_version);
}

} // namespace serialization
} // namespace boost


#endif // NEW_BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
