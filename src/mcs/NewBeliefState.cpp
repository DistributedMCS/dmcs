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
 * @file   NewBeliefState.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  16 10:22:30 2011
 * 
 * @brief  
 * 
 * 
 */

#include <list>
#include "mcs/NewBeliefState.h"

namespace dmcs {

NewBeliefState::NewBeliefState()
{ }



NewBeliefState::NewBeliefState(std::size_t n, bool setall)
  : status_bit(n), 
    value_bit(n)
{
  if (setall)
    {
      status_bit.set();
      value_bit.set();
    }
}



NewBeliefState::NewBeliefState(std::size_t no_bs, std::size_t bs_size, bool setall)
  : status_bit(no_bs * (bs_size+1)), 
    value_bit(no_bs * (bs_size+1))
{
  if (setall)
    {
      status_bit.set();
      value_bit.set();
    }  
}



NewBeliefState::NewBeliefState(const NewBeliefState& bs)
{
  assert (bs.status_bit.size() == bs.value_bit.size());
  status_bit = bs.status_bit;
  value_bit = bs.value_bit;
}



NewBeliefState&
NewBeliefState::operator= (const NewBeliefState& bs)
{
  if (this != &bs)
    {
      assert (bs.status_bit.size() == bs.value_bit.size());
      status_bit = bs.status_bit;
      value_bit = bs.value_bit;
    }
  
  return *this;
}


// not an associative operator
bool
NewBeliefState::consistent_with(const NewBeliefState& bm,
				std::size_t id,
				const std::vector<std::size_t>& starting_offsets)
{
  assert (status_bit.size() == bm.status_bit.size());
  assert (value_bit.size() == bm.value_bit.size());

  if (isEpsilon(id, starting_offsets) || bm.isEpsilon(id, starting_offsets))
    return true;

  std::size_t start_bit = starting_offsets[id];
  std::size_t end_bit;

  if (id == starting_offsets.size() - 1)
    {
      end_bit = status_bit.size();
    }
  else
    {
      end_bit = starting_offsets[id+1];
    }

  // check in range [start_bit, end_bit)
  std::size_t pos = bm.status_bit.get_next(start_bit);
  
  do
    {
      if (test(pos) != bm.test(pos))
	{
	  return false;
	}
      pos = bm.status_bit.get_next(pos);
    }
  while (pos != 0 && pos < end_bit);

  return true;
}



std::ostream&
NewBeliefState::print(std::ostream& os) const
{
  std::size_t count = status_bit.count();
  if (count == 0)
    {
      os << "[ ]";
      return os;
    }

  os << "[";
  std::size_t bit = status_bit.get_first();
  do
    {
      if (!value_bit.test(bit))
	{
	  os << "-";
	}
      else
	{
	  os << " ";
	}
      os << bit;

      bit = status_bit.get_next(bit);
      
      if (bit)
	{
	  os << ", ";
	}
    }
  while (bit);

  return os << "]";
}



bool
combine(NewBeliefState& s,
	const NewBeliefState& t,
	const std::vector<std::size_t>& starting_offsets,
	const std::vector<BitMagic*>& masks)
{
  assert (masks.size() > 0);
  assert (s.size() == t.size());
  assert (s.size() == (masks[0])->size());
  assert (starting_offsets.size() == masks.size());

  if (starting_offsets.size() > 1)
    {
      assert (s.size() == starting_offsets[starting_offsets.size() - 1] + starting_offsets[1] - starting_offsets[0]);
    }

  std::list<std::size_t> tobe_combined;
  BitMagic mask_check_consistency(s.size());
  BitMagic mask_combine(s.size());

  // Prepare 2 masks, then check for consistency once and 
  // combine also just once. This is to minimize coying of NewBeliefState.
  for (std::size_t i = 0; i < starting_offsets.size(); ++i)
    {
      if (!s.isEpsilon(i, starting_offsets) && !t.isEpsilon(i, starting_offsets))
	{
	  mask_check_consistency |= *masks[i];	  
	}
      else if (s.isEpsilon(i, starting_offsets) && (!t.isEpsilon(i, starting_offsets)))
	{
	  mask_combine |= *masks[i];
	}
    }

  // Now check for consistency
  NewBeliefState s1 = s;
  NewBeliefState t1 = t;
  and_bm(s1, mask_check_consistency);
  and_bm(t1, mask_check_consistency);

  if (s1 != t1)
    {
      return false;
    }

  // Now combine
  NewBeliefState s2 = s;
  NewBeliefState t2 = t;
  and_bm(s2, ~mask_combine);
  and_bm(t2, mask_combine);
  s2 = s2 | t2;
  
  s = s | s2;

  return true;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
