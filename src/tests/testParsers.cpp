#include "Theory.h"
#include "CnfGrammar.h"
#include "CnfBuilder.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ParserDirector.h"
#include "DimacsVisitor.h"
#include "BeliefState.h"

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


TheoryPtr theory(new Theory);
BiMapId2Name id2name;
BiMapId2Lid id2lid;
BiMapId2Lid my_id2lid;


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

// void
// print_theory()
// {
//   std::cerr << "Encoding atoms:" << std::endl;

//   for (BiMapId2Name::left_const_iterator it1 = id2name.left.begin(); it1 != id2name.left.end(); ++it1)
//     {
//       std::cerr << it1->second << " --> "
// 		<< it1->first << " --> "
// 		<< id2lid.left.at(it1->first)
// 		<< std::endl;
//     }

//   std::cerr << std::endl;

//   std::cerr << "The theory: " << std::endl;
  
//   for (Theory::const_iterator it2 = theory->begin(); it2 != theory->end(); ++it2)
//     {
//       ClausePtr c = *it2;
//       for (Clause::const_iterator it3 = c->begin(); it3 != c->end(); ++it3)
//        	{
//        	  std::cerr << *it3 << ", ";
//        	}
//       std::cerr << std::endl;
//     }

//   std::cerr << std::endl << "Dimacs style: " << std::endl;

//   std::ostringstream s;
//   DimacsVisitor dv(s);
//   dv.visitTheory(theory, id2lid);
//   std::cerr << s.str() << std::endl;
// }

// void print_clasp_result()
// {
//   std::cerr << std::endl << "Belief sets parsed from clasp result:" << std::endl;
//   for (BeliefSets::const_iterator it = belief_sets->begin(); it != belief_sets->end(); ++it)
//     {
//       BeliefSet bs = *it;
//       std::cerr << "{";
//       for (std::map<int, char>::const_iterator it2 = bs.begin(); it2 != bs.end(); ++it2)
// 	{
// 	  std::cerr << it2->first << it2->second << ", ";
// 	}
//       std::cerr << "}" << std::endl;
//     }
// }



BOOST_AUTO_TEST_CASE( testParsersCnf )
{
  const char* ex = getenv("EXAMPLESDIR");
  // some theory file
  std::string theoryfile(ex);
  theoryfile += "/sat1.txt";
  // some clasp results file
  std::string resultfile(ex);
  resultfile += "/clasp-result.txt";


  CnfBuilder<CnfGrammar> builder(theory, id2name);
  ParserDirector<CnfGrammar> parser_director;

  parser_director.setBuilder(&builder);
  parser_director.parse(theoryfile);

  BiMapId2Lid sat_id2lid = builder.getLocalSATId();
  
  Neighbors nbs;
  std::size_t system_size = 1;

  Context context(1, theory, id2name, sat_id2lid, my_id2lid, system_size, nbs, true);
  BeliefStatesPtr bs(new BeliefStates(system_size));
  ClaspResultBuilder<ClaspResultGrammar> builder2(context, bs);
  ParserDirector<ClaspResultGrammar> parser_director2;
  parser_director2.setBuilder(&builder2);
  parser_director2.parse(resultfile);
  // print_clasp_result();
}
