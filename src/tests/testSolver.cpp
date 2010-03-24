#include "Theory.h"
#include "CnfGrammar.h"
#include "CnfBuilder.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ClaspProcess.h"
#include "ParserDirector.h"
#include "DimacsVisitor.h"
#include "BeliefState.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSolver"
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

using namespace dmcs;


std::size_t system_size = 1;
TheoryPtr theory(new Theory);
BiMapId2Name id2name;
BiMapId2Lid id2lid;
BeliefStatesPtr belief_states(new BeliefStates(system_size));

void
create_local_id()
{
  int i = 1;
  for (BiMapId2Name::left_const_iterator it = id2name.left.begin(); 
       it != id2name.left.end(); ++i, ++it)
    {
      BiMapId2Lid::value_type p(it->first, i);
      id2lid.insert(p);
    }
}

void print_clasp_result()
{
  std::cerr << std::endl << "Belief sets parsed from clasp result:" << std::endl;
  std::cerr << belief_states;
}

BOOST_AUTO_TEST_CASE( testSolverClaspDimacs )
{
  const char* ex = getenv("EXAMPLESDIR");
  std::string dimacsfile(ex);
  dimacsfile += "/sat.txt";

  // get the input
  CnfBuilder<CnfGrammar> builder(theory, id2name);
  ParserDirector<CnfGrammar> parser_director;
  parser_director.setBuilder(&builder);
  parser_director.parse(dimacsfile);
  create_local_id();

  // call clasp

  Neighbors n;
  BiMapId2Lid sat_id2lid;
  Context c(1, theory, id2name, id2lid, id2lid, 1, n, false);
    
  // call clasp
  ClaspProcess cp;
  cp.addOption("-n 0");
  boost::shared_ptr<BaseSolver> solver(cp.createSolver());

  std::cerr << "Calling solver..." << std::endl;
  solver->solve(c, belief_states);

  print_clasp_result();
}

// Local Variables:
// mode: C++
// End:
