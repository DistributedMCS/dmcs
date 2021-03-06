#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "process/ClaspProcess.h"
#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

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


BOOST_AUTO_TEST_CASE( testClaspResultBig )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string resultfile(ex);
  resultfile += "/clasp-result-big.txt";

  std::size_t system_size = 3;
  std::size_t k = 1;

  SignaturePtr sig(new Signature);
  RulesPtr kb(new Rules);
  BridgeRulesPtr br(new BridgeRules);
  NeighborListPtr nb(new NeighborList);

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

  Context context(k, system_size, sig, kb, br, nb);
  BeliefStateListPtr bs(new BeliefStateList);

  SignaturePtr foosig(new Signature);
  ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*foosig));

  ClaspResultBuilder<ClaspResultGrammar> builder(mixed_sig, bs, system_size);
  ParserDirector<ClaspResultGrammar> parser_director;
  parser_director.setBuilder(&builder);
  parser_director.parse(resultfile);

  // 10 ... satisfiable
  BOOST_CHECK_EQUAL(bs->size(), (std::size_t)2045);
}


BOOST_AUTO_TEST_CASE( testClaspResultSmall )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string resultfile(ex);
  resultfile += "/dimacs-small-result.txt";

  std::size_t system_size = 3;
  std::size_t k = 1;

  SignaturePtr sig(new Signature);
  RulesPtr kb(new Rules);
  BridgeRulesPtr br(new BridgeRules);
  NeighborListPtr nb(new NeighborList);

  // setup basic signature from 'a' (==1) to 'g' (==7) for context k
  for (char c = 'a'; c <= 'g'; ++c)
    {
      std::size_t i = c - 'a' + 1;
      std::string s;
      s += c;
      Symbol sym(s, k, i, i);
      sig->insert(sym);
      k = (k + 1) > 3 ? 1 : k + 1;
    }

  Context context(k, system_size, sig, kb, br, nb);
  BeliefStateListPtr bs(new BeliefStateList);

  SignaturePtr foosig(new Signature);
  ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*foosig));

  ClaspResultBuilder<ClaspResultGrammar> builder(mixed_sig, bs, system_size); 
  ParserDirector<ClaspResultGrammar> parser_director;
  parser_director.setBuilder(&builder);
  parser_director.parse(resultfile);

  // 10 ... satisfiable
  BOOST_CHECK_EQUAL(bs->size(), (std::size_t)10);
}


#if 0
BOOST_AUTO_TEST_CASE( testClaspSolver )
{
  ClaspProcess cp;
  cp.addOption("-n 0");
  boost::shared_ptr<BaseSolver> solver(cp.createSolver());

  BeliefStateListPtr belief_states(new BeliefStateList);

  ClausePtr c1(new Clause);
  ClausePtr c2(new Clause);

  c1->push_back(1);
  c1->push_back(-3);
  c2->push_back(-1);
  c2->push_back(2);
  c2->push_back(3);
  
  TheoryPtr t(new Theory);
  t->push_back(c1);
  t->push_back(c2);

  solver->solve(*ctx, local_belief_states, theory, V);


  // 10 ... satisfiable
  BOOST_CHECK_EQUAL(clasp.close(), 10);
}
#endif
