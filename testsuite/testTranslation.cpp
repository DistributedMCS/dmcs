#include "mcs/Rule.h"
#include "mcs/Signature.h"

#include "dmcs/QueryPlan.h"

#include "loopformula/DimacsVisitor.h"
#include "loopformula/LocalLoopFormulaBuilder.h"
#include "loopformula/CNFLocalLoopFormulaBuilder.h"
#include "loopformula/EquiCNFLocalLoopFormulaBuilder.h"
#include "loopformula/LoopFormulaDirector.h"

#include "parser/BRGrammar.h"
#include "parser/BridgeRulesBuilder.h"
#include "parser/LocalKBBuilder.h"
#include "parser/ParserDirector.h"
#include "parser/PropositionalASPGrammar.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testTranslation"
#include <boost/test/unit_test.hpp>

#include <fstream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testCNFTranslation )
{
  const std::size_t system_size = 4;

  // create signature
  SignaturePtr sig(new Signature);

  sig->insert(Symbol("a",1,1,1));
  sig->insert(Symbol("b",1,2,2));
  //sig->insert(Symbol("c3",3,1,1));
  //sig->insert(Symbol("d3",3,2,2));
  //sig->insert(Symbol("e3",3,3,3));
  //sig->insert(Symbol("f4",4,3,1));
  //sig->insert(Symbol("g4",4,2,2));

  std::size_t contextId = 2;

  RulesPtr local_kb(new Rules);
  BridgeRulesPtr bridge_rules(new BridgeRules);

  const char* ex = getenv("EXAMPLESDIR");

  assert (ex != 0);

  std::string kb_file(ex);
  kb_file += "/rules.txt";
  std::string br_file(ex);
  br_file += "/bridgeRules.txt";

  

  // parse KB
  LocalKBBuilder<PropositionalASPGrammar> builder1(local_kb, sig);
  ParserDirector<PropositionalASPGrammar> parser_director;
  parser_director.setBuilder(&builder1);
  parser_director.parse(kb_file);


  NeighborListPtr neighbor_list(new NeighborList);

  SignatureVecPtr global_sigs(new SignatureVec(system_size));
  global_sigs->at(0) = sig;
  global_sigs->at(1) = sig;
  global_sigs->at(2) = sig;
  global_sigs->at(3) = sig;

  //parse BR
  BridgeRulesBuilder<BRGrammar> builder_br(contextId, bridge_rules, neighbor_list, global_sigs);
  ParserDirector<BRGrammar> parser_director_br;
  parser_director_br.setBuilder(&builder_br);
  parser_director_br.parse(br_file);

  // get Size of local Sig
  const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);

  SignatureByCtx::const_iterator low = local_sig.lower_bound(contextId);       
  SignatureByCtx::const_iterator up = local_sig.upper_bound(contextId);

  std::size_t size = std::distance(low,up);

  //construct loop formulae
  CNFLocalLoopFormulaBuilder lf_builder(sig,size);
  //  EquiCNFLocalLoopFormulaBuilder lf_builder(sig,size);
  LoopFormulaDirector director;
  director.setBuilder(&lf_builder);
  director.construct(local_kb, bridge_rules);
 
  // use Dimacs visitor to check the result 
  TheoryPtr formula;
  formula = lf_builder.getFormula();
  SignaturePtr resultSig;
  resultSig = lf_builder.getSignature();
 
  std::fstream fs("cnfLoopformula.txt",std::ios::out);
  //  std::fstream fs("equicnfLoopformula.txt",std::ios::out);

  BOOST_REQUIRE(fs.is_open());


  DimacsVisitor v(fs);
  v.visitTheory(formula, resultSig->size());
  fs.close();

}
//  make -e CPPFLAGS="-DDEBUG"    in src directory
