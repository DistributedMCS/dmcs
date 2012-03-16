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

#include <iostream>
#include <iomanip>

namespace dmcs {

class QueryID
{
public:
  // can now afford up to 2^4 = 16 different types
  static const std::size_t TYPE_SHUTDOWN = 1;
  static const std::size_t TYPE_REQUEST  = 2;
  static const std::size_t TYPE_ANSWER   = 3;

  static QueryID* instance();

  std::size_t
  QUERY_ORDER_SHIFT() const;

  std::size_t
  LOCAL_CONTEXT_SHIFT() const;

  std::size_t
  NEIGHBOR_OFFSET_SHIFT() const;

  std::size_t
  NEIGHBOR_ID_SHIFT() const;

  std::size_t
  QUERY_TYPE_MASK() const;

  std::size_t
  QUERY_ORDER_MASK() const;

  std::size_t
  LOCAL_CONTEXT_MASK() const;

  std::size_t
  NEIGHBOR_OFFSET_MASK() const;

  std::size_t
  NEIGHBOR_ID_MASK() const;

  std::ostream&
  print(std::ostream& os) const;

protected:
  QueryID();

private:
  void
  setupMask(std::size_t& mask,
	    std::size_t mask_beg, 
	    std::size_t mask_shift);

private:
  std::size_t query_order_shift;
  std::size_t local_context_shift;
  std::size_t neighbor_offset_shift;
  std::size_t neighbor_id_shift;

  std::size_t query_type_mask;
  std::size_t query_order_mask;
  std::size_t local_context_mask;
  std::size_t neighbor_offset_mask;
  std::size_t neighbor_id_mask;

  static QueryID* _instance;
};

// Helper functions

inline void
print_query_id(const std::size_t qid)
{
  std::cerr << std::setfill('0') << std::hex << std::setw(16) << qid << std::endl;
}


inline std::size_t
query_id(const std::size_t ctx_id, 
	 const std::size_t query_order)
{
  return (QueryID::TYPE_REQUEST
	  | (query_order << QueryID::instance()->QUERY_ORDER_SHIFT())
	  | (ctx_id << QueryID::instance()->LOCAL_CONTEXT_SHIFT()));
}


inline std::size_t
query_id(const std::size_t ctx_id, 
	 const std::size_t neighbor_id, 
	 const std::size_t query_order)
{
  return (QueryID::TYPE_REQUEST
	  | (query_order << QueryID::instance()->QUERY_ORDER_SHIFT())
	  | (ctx_id << QueryID::instance()->LOCAL_CONTEXT_SHIFT())
	  | (neighbor_id << QueryID::instance()->NEIGHBOR_ID_SHIFT()));
}



inline std::size_t
query_id(const std::size_t query_type,
	 const std::size_t ctx_id, 
	 const std::size_t neighbor_id,
	 const std::size_t neighbor_offset,
	 const std::size_t query_order)
{
  return (query_type
	  | (query_order << QueryID::instance()->QUERY_ORDER_SHIFT()) 
	  | (ctx_id << QueryID::instance()->LOCAL_CONTEXT_SHIFT()) 
	  | (neighbor_offset << QueryID::instance()->NEIGHBOR_OFFSET_SHIFT())
	  | (neighbor_id << QueryID::instance()->NEIGHBOR_ID_SHIFT()));
}



inline std::size_t
shutdown_query_id()
{
  return QueryID::TYPE_SHUTDOWN;
}



inline std::size_t
shutdown_query_id(const std::size_t ctx_id,
		  const std::size_t neighbor_id)
{
  return (QueryID::TYPE_SHUTDOWN
	  | (ctx_id << QueryID::instance()->LOCAL_CONTEXT_SHIFT())
	  | (neighbor_id << QueryID::instance()->NEIGHBOR_ID_SHIFT()));
}


inline bool
is_shutdown(std::size_t qid)
{
  return (qid & QueryID::instance()->QUERY_TYPE_MASK()) == QueryID::TYPE_SHUTDOWN;
}



inline bool
is_request(std::size_t qid)
{
  return (qid & QueryID::instance()->QUERY_TYPE_MASK()) == QueryID::TYPE_REQUEST;
}



inline bool
is_answer(std::size_t qid)
{
  return (qid & QueryID::instance()->QUERY_TYPE_MASK()) == QueryID::TYPE_ANSWER;
}



inline std::size_t
ctxid_from_qid(std::size_t qid)
{
  return ((qid & QueryID::instance()->LOCAL_CONTEXT_MASK()) >> QueryID::instance()->LOCAL_CONTEXT_SHIFT());
}



// just a synonym, for more intuitive use at the request dispatcher
inline std::size_t
invoker_from_qid(std::size_t qid)
{
  return ctxid_from_qid(qid);
}



inline std::size_t
neighbor_offset_from_qid(std::size_t qid)
{
  return ((qid & QueryID::instance()->NEIGHBOR_OFFSET_MASK()) >> QueryID::instance()->NEIGHBOR_OFFSET_SHIFT());
}



inline std::size_t
neighbor_id_from_qid(std::size_t qid)
{
  return ((qid & QueryID::instance()->NEIGHBOR_ID_MASK()) >> QueryID::instance()->NEIGHBOR_ID_SHIFT());
}


// just a synonym, for more intuitive use at the message receiver
inline std::size_t
receiver_from_qid(std::size_t qid)
{
  return neighbor_id_from_qid(qid);
}


inline void
set_neighbor_offset(std::size_t& qid, const std::size_t n_offset)
{
  qid |= (n_offset << QueryID::instance()->NEIGHBOR_OFFSET_SHIFT());
}



inline void
set_neighbor_id(std::size_t& qid, const std::size_t nid)
{
  qid |= (nid << QueryID::instance()->NEIGHBOR_ID_SHIFT());
}


inline void
unset_neighbor_offset(std::size_t& qid)
{
  qid &= ~(QueryID::instance()->NEIGHBOR_OFFSET_MASK());
}



inline std::size_t
qorder_from_qid(const std::size_t qid)
{
  return ((qid & QueryID::instance()->QUERY_ORDER_MASK()) >> QueryID::instance()->QUERY_ORDER_SHIFT());
}

} // namespace dmcs

#endif // QUERY_ID_H

// Local Variables:
// mode: C++
// End:
