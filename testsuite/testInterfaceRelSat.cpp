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

void
print_new_conflicts(const ConflictBufVecPtr& learned_conflicts, const ConflictBufIterVecPtr& new_conflicts_beg)
{
  std::size_t off = 0;
  for (ConflictBufVec::const_iterator it = learned_conflicts->begin(); 
       it != learned_conflicts->end(); ++it, ++off)
    {
      ConflictBufPtr cb = *it;
      ConflictBuf::iterator jt = (*new_conflicts_beg)[off];
      if (jt == cb->end())
	{
	  DMCS_LOG_DEBUG("offset[" << off << "]: NONE");
	}
      else
	{
	  DMCS_LOG_DEBUG("offset[" << off << "]:");
	  for (; jt != cb->end(); ++jt)
	    {
	      DMCS_LOG_DEBUG(**jt);
	    }
	}
    }
}


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



#warning disabled testcase

#if 0

  RelSatSolver rss(false, my_id, theory, sig, localV, c2o.get(), system_size, mg.get(), dsn.get(), srn.get(), 42);

  DMCS_LOG_DEBUG("Original theory: ");
  rss.print_local_theory();

  std::string clauses_list1 = "1 -2\n 1 3\n -3 4\n 4 6\n 5 -6\n -5 6\n";
  std::stringstream iss(clauses_list1);
  ClauseList cl1;
  iss >> cl1;

  std::string clause_list2 = "1 2\n 3 -4\n -3 4\n 5 -6\n 5 6\n";
  iss.clear();
  iss.str(clause_list2);
  ClauseList cl2;
  iss >> cl2;

  std::string clause_list3 = "-3 4\n -5 -6\n";
  iss.clear();
  iss.str(clause_list3);
  ClauseList cl3;
  iss >> cl3;

  DMCS_LOG_DEBUG("Going to learn");

  rss.collect_learned_clauses(cl1);

  DMCS_LOG_DEBUG("After learning first set of conflicts: ");
  ConflictBufVecPtr learned_conflicts = rss.getLearnedConflicts();
  DMCS_LOG_DEBUG(*learned_conflicts);

  rss.collect_learned_clauses(cl2);

  DMCS_LOG_DEBUG("After learning second set of conflicts: ");
  learned_conflicts = rss.getLearnedConflicts();
  DMCS_LOG_DEBUG(*learned_conflicts);


  DMCS_LOG_DEBUG("New learned conflicts:");
  ConflictBufIterVecPtr new_conflicts_beg = rss.getNewLearnedConflicts();
  print_new_conflicts(learned_conflicts, new_conflicts_beg);

  rss.collect_learned_clauses(cl3);

  DMCS_LOG_DEBUG("After learning second set of conflicts: ");
  learned_conflicts = rss.getLearnedConflicts();
  DMCS_LOG_DEBUG(*learned_conflicts);


  DMCS_LOG_DEBUG("New learned conflicts:");
  new_conflicts_beg = rss.getNewLearnedConflicts();
  print_new_conflicts(learned_conflicts, new_conflicts_beg);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

#endif
}
