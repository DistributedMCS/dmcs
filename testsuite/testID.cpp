#include <iostream>
#include "mcs/QueryID.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBlockingAsio"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( testQueryID )
{
  std::size_t qid = dmcs::query_id(24, 21);
  std::size_t ctx_id = dmcs::ctxid_from_qid(qid);
  std::size_t qorder = dmcs::qorder_from_qid(qid);

  BOOST_CHECK_EQUAL(ctx_id, 24);
  BOOST_CHECK_EQUAL(qorder, 21);
}
