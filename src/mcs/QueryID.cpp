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
 * @file   QueryID.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Dec  22 22:10:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/QueryID.h"

namespace dmcs {

QueryID* QueryID::_instance = 0;

QueryID::QueryID()
{
  std::size_t s = sizeof(std::size_t)*8;
  
  neighbor_offset_shift = s/2;
  local_context_shift = 3*s/4;

  local_context_mask = 0;
  neighbor_offset_mask = 0;
  query_order_mask = 0;

  for (std::size_t i = local_context_shift; i < s; ++i)
    {
      local_context_mask |= (std::size_t)1 << i;
    }

  for (std::size_t i = neighbor_offset_shift; i < local_context_shift; ++i)
    {
      neighbor_offset_mask |= (std::size_t)1 << i;
    }
  
  for (std::size_t i = 0; i < neighbor_offset_shift; ++i)
    {
      query_order_mask |= (std::size_t)1 << i;
    }

  local_context_mask = ~query_order_mask;
}

QueryID* 
QueryID::instance()
{
  if (_instance == 0)
    {
      _instance = new QueryID;
    }
  return _instance;
}


std::size_t
QueryID::LOCAL_CONTEXT_SHIFT() const
{
  return local_context_shift;
}


std::size_t
QueryID::NEIGHBOR_OFFSET_SHIFT() const
{
  return neighbor_offset_shift;
}


std::size_t
QueryID::LOCAL_CONTEXT_MASK() const
{
  return local_context_mask;
}


std::size_t
QueryID::NEIGHBOR_OFFSET_MASK() const
{
  return neighbor_offset_mask;
}

std::size_t
QueryID::QUERY_ORDER_MASK() const
{
  return query_order_mask;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
