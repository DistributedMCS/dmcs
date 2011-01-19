#include "process/ClaspProcess.h"
#include "parser/ClaspResultOnlineParser.h"
#include "dmcs/Log.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBlockingClasp"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testBlockingClasp )
{
  init_loggers("testBlockingClasp");

  // Prepare some signatures
  const SignaturePtr sig(new Signature);
  const SignaturePtr gsig(new Signature);

  sig->insert(Symbol("a1", 1, 1, 1));
  sig->insert(Symbol("b1", 1, 2, 2));
  sig->insert(Symbol("c1", 1, 3, 3));
  sig->insert(Symbol("d1", 1, 4, 4));
  sig->insert(Symbol("e1", 1, 5, 5));
  sig->insert(Symbol("f1", 1, 6, 6));
  sig->insert(Symbol("g1", 1, 7, 7));
  sig->insert(Symbol("h1", 1, 8, 8));
  sig->insert(Symbol("i1", 1, 9, 9));
  sig->insert(Symbol("j1", 1, 10, 10));

  gsig->insert(Symbol("a2", 2, 11, 1));
  gsig->insert(Symbol("b2", 2, 12, 2));
  gsig->insert(Symbol("c2", 2, 13, 3));
  gsig->insert(Symbol("d2", 2, 14, 4));
  gsig->insert(Symbol("e2", 2, 15, 5));

  gsig->insert(Symbol("a3", 3, 16, 1));
  gsig->insert(Symbol("b3", 3, 17, 2));
  gsig->insert(Symbol("c3", 3, 18, 3));
  gsig->insert(Symbol("d3", 3, 19, 4));
  gsig->insert(Symbol("e3", 3, 20, 5));

  ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*gsig));

  ClaspProcess clasp;

  clasp.addOption("0");

  clasp.spawn();

  std::ostream& os = clasp.getOutput();
  os << "p cnf 20 0" << std::endl;
 
  clasp.endoffile();

  std::istream& result = clasp.getInput();

  ClaspResultOnlineParser crop(result, mixed_sig, 3);
  
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
	  sleep(1);

#ifdef DEBUG
	  std::cerr << "read " << count << std::endl;
#endif
	}
    }
  while (belief_state != boost::shared_ptr<BeliefState>() &&
	 count < 3);
}
