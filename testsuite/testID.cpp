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

BOOST_AUTO_TEST_CASE ( testQueryID )
{
  std::size_t qid = dmcs::query_id(21, 24, 27);
  std::size_t ctx_id = dmcs::ctxid_from_qid(qid);
  std::size_t n_offset = dmcs::neighbor_offset_from_qid(qid);
  std::size_t qorder = dmcs::qorder_from_qid(qid);

  BOOST_CHECK_EQUAL(ctx_id, 21);
  BOOST_CHECK_EQUAL(n_offset, 24);
  BOOST_CHECK_EQUAL(qorder, 27);

  qid = dmcs::query_id(12, 42);
  ctx_id = dmcs::ctxid_from_qid(qid);
  n_offset = dmcs::neighbor_offset_from_qid(qid);
  qorder = dmcs::qorder_from_qid(qid);

  BOOST_CHECK_EQUAL(ctx_id, 12);
  BOOST_CHECK_EQUAL(n_offset, 0);
  BOOST_CHECK_EQUAL(qorder, 42);

}

// Local Variables:
// mode: C++
// End:
