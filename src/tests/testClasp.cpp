#include "ClaspProcess.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ParserDirector.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testClasp"
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testClaspDimacs )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string dimacsfile(ex);
  dimacsfile += "/dimacs.txt";

  ClaspProcess clasp;

  clasp.addOption("0");
  clasp.addOption(dimacsfile);

  clasp.spawn();
  clasp.endoffile();

  std::istream& result = clasp.getInput();

  std::string resultfile(ex);
  resultfile += "/clasp-result.txt";
  std::fstream fresult(resultfile.c_str());

  std::string resline1;
  std::string resline2;

  while (!fresult.eof() && !result.eof())
    {
      std::getline(fresult, resline1);
      std::getline(result, resline2);

      if (!resline1.empty() && resline1[0] == 'c')
	{
          ///@todo BOOST_WARN_EQUAL does not work *grml*
          BOOST_WARN_EQUAL(resline1, resline2);
        }
      else
	{
          BOOST_CHECK_EQUAL(resline1, resline2);
        }
    }

  BOOST_CHECK(fresult.eof() && result.eof());

  // 10 ... satisfiable
  BOOST_CHECK_EQUAL(clasp.close(), 10);
}


BOOST_AUTO_TEST_CASE( testClaspResult )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string resultfile(ex);
  resultfile += "/clasp-result-big.txt";

  std::size_t system_size = 3;
  std::size_t k = 1;

  QueryPlanPtr qp(new QueryPlan);
  SignaturePtr sig(new Signature);
  RulesPtr kb(new Rules);
  BridgeRulesPtr br(new BridgeRules);

  // setup basic signature from 'a' (==1) to 'k' (==11) for context k
  for (char c = 'a'; c <= 'k'; ++c)
    {
      std::size_t i = c - 'a' + 1;
      std::string s;
      s += c;
      Symbol sym(s, k, i, i);
      sig->insert(sym);
      k = (k + 1) > 3 ? 1 : k + 1;
    }

  Context context(k, system_size, sig, qp, kb, br);
  BeliefStatesPtr bs(new BeliefStates(system_size));

  ClaspResultBuilder<ClaspResultGrammar> builder(context, bs);
  ParserDirector<ClaspResultGrammar> parser_director;
  parser_director.setBuilder(&builder);
  parser_director.parse(resultfile);

  // 10 ... satisfiable
  BOOST_CHECK_EQUAL(bs.belief_states_ptr->belief_states.size(), (std::size_t)2045);
}
