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

#include "mcs/NewBeliefState.h"

namespace dmcs {

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

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
