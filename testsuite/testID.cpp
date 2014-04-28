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
 * @file   testID.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  12 10:45:16 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/QueryID.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testID"
#include <boost/test/unit_test.hpp>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testQueryID )
{
  std::cerr << "QUERY_ORDER_SHIFT     = " << QueryID::instance()->QUERY_ORDER_SHIFT() << std::endl;
  std::cerr << "LOCAL_CONTEXT_SHIFT   = " << QueryID::instance()->LOCAL_CONTEXT_SHIFT() << std::endl;
  std::cerr << "NEIGHBOR_OFFSET_SHIFT = " << QueryID::instance()->NEIGHBOR_OFFSET_SHIFT() << std::endl;
  std::cerr << "NEIGHBOR_ID_SHIFT     = " << QueryID::instance()->NEIGHBOR_ID_SHIFT() << std::endl;

  print_query_id(QueryID::instance()->QUERY_TYPE_MASK());
  print_query_id(QueryID::instance()->QUERY_ORDER_MASK());
  print_query_id(QueryID::instance()->LOCAL_CONTEXT_MASK());
  print_query_id(QueryID::instance()->NEIGHBOR_OFFSET_MASK());
  print_query_id(QueryID::instance()->NEIGHBOR_ID_MASK());
  std::size_t qid = query_id(QueryID::TYPE_REQUEST, 21, 20, 24, 27);
  print_query_id(qid);

  bool is_s = shutdown(qid);
  bool is_r = is_request(qid); 
  std::size_t ctx_id = ctxid_from_qid(qid);
  std::size_t n_offset = neighbor_offset_from_qid(qid);
  std::size_t qorder = qorder_from_qid(qid);

  BOOST_CHECK_EQUAL(is_s, false);
  BOOST_CHECK_EQUAL(is_r, true);
  BOOST_CHECK_EQUAL(ctx_id, 21);
  BOOST_CHECK_EQUAL(n_offset, 24);
  BOOST_CHECK_EQUAL(qorder, 27);

  qid = query_id(12, 42);
  is_s = shutdown(qid);
  ctx_id = ctxid_from_qid(qid);
  n_offset = neighbor_offset_from_qid(qid);
  qorder = qorder_from_qid(qid);

  BOOST_CHECK_EQUAL(is_s, false);
  BOOST_CHECK_EQUAL(ctx_id, 12);
  BOOST_CHECK_EQUAL(n_offset, 0);
  BOOST_CHECK_EQUAL(qorder, 42);

  std::size_t old_qid = qid;

  set_neighbor_offset(qid, 30);
  n_offset = neighbor_offset_from_qid(qid);

  BOOST_CHECK_EQUAL(n_offset, 30);
  BOOST_CHECK_EQUAL(qid == old_qid, false);

  unset_neighbor_offset(qid);
  BOOST_CHECK_EQUAL(qid, old_qid);
}

// Local Variables:
// mode: C++
// End:
