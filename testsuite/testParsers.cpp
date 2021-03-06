#include "dmcs/QueryPlan.h"

#include "parser/LocalKBBuilder.h"
#include "parser/ParserDirector.h"
#include "parser/PropositionalASPGrammar.h"

#include "mcs/BeliefState.h"
#include "mcs/Rule.h"
#include "mcs/Signature.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <set>

#include <fstream>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testParsers"
#include <boost/test/unit_test.hpp>

using namespace dmcs;


BOOST_AUTO_TEST_CASE( testParsersKB )
{
  const char* ex = getenv("EXAMPLESDIR");

  assert (ex != 0);

  std::size_t myid = 1;

  // some theory file
  std::string filename_local_kb(ex);
  filename_local_kb += "/diamond-test-1.lp";

  std::string filename_topo(ex);
  filename_topo += "/diamond-test.opt";

  QueryPlanPtr query_plan(new QueryPlan);

  query_plan->read_graph(filename_topo);

  SignaturePtr sig(new Signature);
  *sig = query_plan->getSignature(myid);

  BOOST_CHECK_EQUAL(sig->size(), 8);

  RulesPtr local_kb(new Rules);

  LocalKBBuilder<PropositionalASPGrammar> builder(local_kb, sig);
  ParserDirector<PropositionalASPGrammar> parser_director;
  parser_director.setBuilder(&builder);
  parser_director.parse(filename_local_kb);

  BOOST_CHECK_EQUAL(local_kb->size(), 7);
}


BOOST_AUTO_TEST_CASE( testParsersBR )
{
}
