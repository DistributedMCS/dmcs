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
 * @file   BeliefStateOffset.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Dec  25 16:44:21 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefStateOffset.h"

namespace dmcs {

BeliefStateOffset* BeliefStateOffset::_instance = 0;

BeliefStateOffset::BeliefStateOffset(std::size_t no_bs, std::size_t size_bs)
  : no_blocks(no_bs),
    size_belief_state(size_bs),
    starting_offsets(no_bs, 0)
{
  assert (no_bs > 0 && size_bs > 0);

  for (std::size_t i = 0; i < no_bs; ++i)
    {
      if (i > 0)
	{
	  starting_offsets[i] = starting_offsets[i-1] + size_bs + 1;
	}
      BitMagic* m = new BitMagic(no_bs * (size_bs+1));
      for (std::size_t j = starting_offsets[i]; 
	   j < starting_offsets[i] + size_bs; ++j)
	{
	  m->set_bit(j);
	}
      masks.push_back(m);
    }
 }



BeliefStateOffset*
BeliefStateOffset::create(std::size_t no_bs, std::size_t size_bs)
{
  assert (_instance == 0);
  _instance = new BeliefStateOffset(no_bs, size_bs);
  return _instance;
}



BeliefStateOffset*
BeliefStateOffset::instance()
{
  assert (_instance != 0);
  return _instance;
}


std::vector<BitMagic*>&
BeliefStateOffset::getMasks()
{
  return masks;
}

std::vector<std::size_t>&
BeliefStateOffset::getStartingOffsets()
{
  return starting_offsets;
}


std::size_t
BeliefStateOffset::NO_BLOCKS() const
{
  return no_blocks;
}


std::size_t
BeliefStateOffset::SIZE_BS() const
{
  return size_belief_state;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
