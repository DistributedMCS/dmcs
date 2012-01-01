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
 * @file   QueryID.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Dec  22 16:25:44 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef QUERY_ID_H
#define QUERY_ID_H

#include <boost/cstdint.hpp>

namespace dmcs {

class QueryID
{
public:
  static QueryID* instance();

  std::size_t
  LOCAL_CONTEXT_SHIFT() const;

  std::size_t
  NEIGHBOR_OFFSET_SHIFT() const;

  std::size_t
  LOCAL_CONTEXT_MASK() const;

  std::size_t
  NEIGHBOR_OFFSET_MASK() const;

  std::size_t
  QUERY_ORDER_MASK() const;

protected:
  QueryID();

private:
  std::size_t local_context_shift;
  std::size_t neighbor_offset_shift;

  std::size_t local_context_mask;
  std::size_t neighbor_offset_mask;
  std::size_t query_order_mask;

  static QueryID* _instance;
};

// Helper functions

inline std::size_t
query_id(const std::size_t ctx_id, 
	 const std::size_t query_order)
{
  return ((ctx_id << QueryID::instance()->LOCAL_CONTEXT_SHIFT()) | query_order);
}



inline std::size_t
query_id(const std::size_t ctx_id, 
	 const std::size_t neighbor_offset,
	 const std::size_t query_order)
{
  return ((ctx_id << QueryID::instance()->LOCAL_CONTEXT_SHIFT()) 
	  | (neighbor_offset << QueryID::instance()->NEIGHBOR_OFFSET_SHIFT())
	  | query_order);
}


inline std::size_t
ctxid_from_qid(std::size_t query_id)
{
  return ((query_id & QueryID::instance()->LOCAL_CONTEXT_MASK()) >> QueryID::instance()->LOCAL_CONTEXT_SHIFT());
}



inline std::size_t
neighbor_offset_from_qid(std::size_t query_id)
{
  return ((query_id & QueryID::instance()->NEIGHBOR_OFFSET_MASK()) >> QueryID::instance()->NEIGHBOR_OFFSET_SHIFT());
}



inline void
set_neighbor_offset(std::size_t& query_id, const std::size_t n_offset)
{
  query_id |= (n_offset << QueryID::instance()->NEIGHBOR_OFFSET_SHIFT());
}



inline std::size_t
qorder_from_qid(const std::size_t query_id)
{
  return (query_id & QueryID::instance()->QUERY_ORDER_MASK());
}

} // namespace dmcs

#endif // QUERY_ID_H

// Local Variables:
// mode: C++
// End:
