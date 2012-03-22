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



void
QueryID::setupMask(std::size_t& mask,
		   std::size_t mask_beg, 
		   std::size_t mask_end)
{
  mask = 0;
  for (std::size_t i = mask_beg; i < mask_end; ++i)
    {
      mask |= (std::size_t)1 << i;
    } 
}



QueryID::QueryID()
{
  // for a 64-bit integer:
  //  4 bits for query type
  // 30 bits for query order
  // 10 bits for context id
  // 10 bits for neighbor offset
  // 10 bits for neighbor id
  // We don't expect more than 2^10 contexts

  std::size_t s = sizeof(std::size_t)*8;
  std::size_t query_order_length = 30;

  query_order_shift = 4;
  while ((s - query_order_shift - query_order_length) % 3 != 0) query_order_shift++;

  std::size_t one_third_left = (s - query_order_shift - query_order_length) / 3;

  local_context_shift = query_order_shift + query_order_length;
  neighbor_offset_shift = local_context_shift + one_third_left;
  neighbor_id_shift = neighbor_offset_shift + one_third_left;

  setupMask(query_type_mask, 0, query_order_shift);
  setupMask(query_order_mask, query_order_shift, local_context_shift);
  setupMask(local_context_mask, local_context_shift, neighbor_offset_shift);
  setupMask(neighbor_offset_mask, neighbor_offset_shift, neighbor_id_shift);
  setupMask(neighbor_id_mask, neighbor_id_shift, s);
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
QueryID::QUERY_ORDER_SHIFT() const
{
  return query_order_shift;
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
QueryID::NEIGHBOR_ID_SHIFT() const
{
  return neighbor_id_shift;
}



std::size_t
QueryID::QUERY_TYPE_MASK() const
{
  return query_type_mask;
}



std::size_t
QueryID::QUERY_ORDER_MASK() const
{
  return query_order_mask;
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
QueryID::NEIGHBOR_ID_MASK() const
{
  return neighbor_id_mask;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
