#include "QueryPlan.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSignature"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testGraphProp )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string graphfile(ex);
  graphfile += "/graph.txt";

  QueryPlanPtr query_plan(new QueryPlan);
  query_plan->read_graph(graphfile);

#if 0
  for (std::size_t i = 0; i < 31; ++i)
    {
      query_plan->print_vertex_info(i);
    }
#endif
}
