// Incrementally parse result from clasp (full vesion)

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testClaspResultOnlineParser"
#include <boost/test/unit_test.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>

#include "parser/ClaspResultOnlineParser.h"

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testClaspResultOnlineParser )
{
  // Prepare some ProxySignatureByLocal:
  // sig  = (a1 1 1 1), (b1 1 2 2), (c1 1 3 3)
  // gsig = (a2 2 4 1)
  
  const SignaturePtr sig(new Signature);
  const SignaturePtr gsig(new Signature);

  sig->insert(Symbol("a1", 1, 1, 1));
  sig->insert(Symbol("b1", 1, 2, 2));
  sig->insert(Symbol("c1", 1, 3, 3));
  gsig->insert(Symbol("a2", 2, 4, 1));
  
  ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*gsig));

  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string resultfile(ex);
  resultfile += "/result.sat";

  std::ifstream claspfile(resultfile.c_str());
  
  ClaspResultOnlineParser crop(claspfile, mixed_sig, 2);
  std::size_t count = 0;
  
  BeliefStatePtr belief_state;
  do
    {
      belief_state = crop.getNextAnswer();
      if (belief_state != boost::shared_ptr<BeliefState>())
	{
#ifdef DEBUG
	  std::cerr << belief_state << std::endl;
#endif
	  count++;
	}
    }
  while (belief_state != boost::shared_ptr<BeliefState>());

  // in this example, there are 8 models computed from clasp 
  BOOST_CHECK_EQUAL(count, 8);
}

