#include "dmcs/Log.h"
#include "relsat-20070104/RelSatHelper.h"
#include "solver/RelSatSolver.h"


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testInterfaceRelSat"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


BOOST_AUTO_TEST_CASE ( testInterfaceRelSat )
{
  init_loggers("testInterfaceRelSat");

  std::size_t my_id = 1;
  std::size_t system_size = 3;
  std::size_t no_nbs = 2;

  // prepare a dummy theory ---------------
  ClausePtr c1(new dmcs::Clause);
  c1->push_back(1);
  c1->push_back(-2);
  c1->push_back(3);

  ClausePtr c2(new dmcs::Clause);
  c2->push_back(-1);
  c2->push_back(2);
  c2->push_back(6);

  TheoryPtr theory(new Theory);
  theory->push_back(c1);
  theory->push_back(c2);

  SignaturePtr sig(new Signature);
  sig->insert(Symbol("a", 1, 1, 1));
  sig->insert(Symbol("b", 1, 2, 2));
  sig->insert(Symbol("c", 2, 3, 1));
  sig->insert(Symbol("d", 2, 4, 2));
  sig->insert(Symbol("e", 3, 5, 1));
  sig->insert(Symbol("f", 3, 6, 2));

  BeliefStatePtr localV(new BeliefState(system_size, BeliefSet()));
  BeliefSet& b1 = (*localV)[0];
  setEpsilon(b1);
  setBeliefSet(b1, 1);
  setBeliefSet(b1, 2);

  BeliefSet& b2 = (*localV)[1];
  setEpsilon(b2);
  setBeliefSet(b2, 1);
  setBeliefSet(b2, 2);

  BeliefSet& b3 = (*localV)[2];
  setEpsilon(b3);
  setBeliefSet(b3, 1);
  setBeliefSet(b3, 2);

  HashedBiMapPtr c2o(new HashedBiMap);
  c2o->insert(Int2Int(2, 0));
  c2o->insert(Int2Int(3, 1));

  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
  MessagingGatewayBCPtr mg = mqf.createMessagingGateway(2421, 5);

  ConcurrentMessageQueuePtr dsn(new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr srn(new ConcurrentMessageQueue);

  RelSatSolver rss(false, my_id, theory, sig, localV, c2o.get(), system_size, mg.get(), dsn.get(), srn.get(), 42);

  DMCS_LOG_DEBUG("Original theory: ");
  rss.print_local_theory();

  std::string clauses_list = "1 -2\n 1 3\n -3 4\n 4 6\n 5 -6\n -5 6\n";
  std::stringstream iss(clauses_list);
  ClauseList cl;
  iss >> cl;

  rss.collect_learned_clauses(cl);

  DMCS_LOG_DEBUG("Learning conflicts: ");
  ConflictVec2Ptr learned_conflicts = rss.getLearnedConflicts();
  DMCS_LOG_DEBUG(learned_conflicts);

  ConflictVec* conflicts = new ConflictVec;

  Conflict* cf1 = new Conflict(system_size, PartialBeliefSet());
  PartialBeliefSet& cf1_0 = (*cf1)[0];
  setBeliefSet(cf1_0, 1);
  setBeliefSet(cf1_0, 2);

  Conflict* cf2 = new Conflict(system_size, PartialBeliefSet());
  PartialBeliefSet& cf2_0 = (*cf2)[0];
  setBeliefSet(cf2_0, 1, PartialBeliefSet::DMCS_FALSE);
  setBeliefSet(cf2_0, 2);

  conflicts->push_back(cf1);
  conflicts->push_back(cf2);

  rss.import_conflicts(conflicts);
  DMCS_LOG_DEBUG("After import conflicts: ");
  rss.print_local_theory();  

  PartialBeliefState* partial_ass = new PartialBeliefState(system_size, PartialBeliefSet());

  PartialBeliefSet& pb0 = (*partial_ass)[0];

  setBeliefSet(pb0, 1, PartialBeliefSet::DMCS_TRUE);

  rss.import_partial_ass(partial_ass);

  DMCS_LOG_DEBUG("After import partial_ass: ");
  rss.print_local_theory();
}
