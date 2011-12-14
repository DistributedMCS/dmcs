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


namespace dmcs {

typedef bm::bvector<> BitMagic;

struct NewBeliefState
{
  // for 3-value assignment
  enum TruthVal
    {
      DMCS_FALSE = 0,
      DMCS_TRUE,
      DMCS_UNDEF
    };

  NewBeliefState(std::size_t n)
    : status_bit(n), 
      value_bit(n)
  { }

  NewBeliefState(const NewBeliefState& bs)
  {
    assert (bs.status_bit.size() == bs.value_bit.size());
    status_bit = bs.status_bit;
    value_bit = bs.value_bit;
  }

  NewBeliefState&
  operator= (const NewBeliefState& bs)
  {
    if (this != &bs)
      {
	assert (bs.status_bit.size() == bs.value_bit.size());
	status_bit = bs.status_bit;
	value_bit = bs.value_bit;
      }

    return *this;
  }

  inline bool
  operator== (const NewBeliefState& bs2)
  {
    return status_bit == bs2.status_bit && value_bit == bs2.value_bit;
  }

  inline bool
  operator!= (const NewBeliefState& bs2)
  {
    return value_bit != bs2.value_bit || status_bit != bs2.status_bit;
  }

  inline bool
  operator< (const NewBeliefState& bs2)
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

  BitMagic status_bit;
  BitMagic value_bit;
};


inline NewBeliefState::TruthVal
test(const NewBeliefState& bs, 
     std::size_t pos)
{
  assert (pos > 0 && pos < bs.size());

  if (bs.status_bit.test(pos))
    {
      if (bs.value_bit.test(pos))
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
set(NewBeliefState& bs, 
    std::size_t pos,
    NewBeliefState::TruthVal val = NewBeliefState::DMCS_TRUE)
{
  assert (pos > 0 && pos < bs.size());
  
  switch (val)
    {
    case NewBeliefState::DMCS_UNDEF:
      {
	bs.status_bit.set_bit(pos, false);
	bs.value_bit.set_bit(pos, false);
	break;
      }
    case NewBeliefState::DMCS_TRUE:
      {
	bs.status_bit.set_bit(pos);
	bs.value_bit.set_bit(pos);
	break;
      }
    case NewBeliefState::DMCS_FALSE:
      {
	bs.status_bit.set_bit(pos);
	bs.value_bit.set_bit(pos, false);
	break;
      }
    }
}


inline bool
isEpsilon(const NewBeliefState& bs, 
	  std::size_t ctx_id,
	  const std::vector<std::size_t>& starting_offset)
{
  return !(bs.status_bit.test(starting_offset[ctx_id]));
}



inline void
setEpsilon(NewBeliefState& bs,
	  std::size_t ctx_id,
	  const std::vector<std::size_t>& starting_offset)
{
  bs.status_bit.set(starting_offset[ctx_id]);
  bs.value_bit.set(starting_offset[ctx_id]); // just want to have it clean
}


} // namespace dmcs


#endif // NEW_BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
