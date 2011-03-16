/* DMCS -- Distributed Nonmonotonic Multi-Context Systems.
 * Copyright (C) 2009, 2010 Minh Dao-Tran, Thomas Krennwallner
 * 
 * This file is part of DMCS.
 *
 *  DMCS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DMCS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DMCS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   RelSatSolver.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  4 17:14:38 2011
 * 
 * @brief  
 * 
 * 
 */


#include "dmcs/Context.h"
#include "mcs/HashedBiMap.h"
#include "mcs/ProxySignatureByLocal.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "solver/BaseSolver.h"
#include "solver/ChoicePoint.h"

#include "relsat-20070104/RelSatHelper.h"
#include "relsat-20070104/SATInstance.h"
#include "relsat-20070104/SATSolver.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#ifndef REL_SAT_SOLVER_H
#define REL_SAT_SOLVER_H


namespace dmcs {

// Wrapper for relsat
class RelSatSolver : public BaseSolver
{
public:
  RelSatSolver(bool il,
	       std::size_t my_id_,
	       std::size_t my_sid,
	       const TheoryPtr& theory_,
	       const SignaturePtr& local_sig_,
	       const BeliefStatePtr& localV_,
	       const HashedBiMap* co,
	       std::size_t system_size_,
	       MessagingGatewayBC* mg_,
	       ConcurrentMessageQueue* dsn,
	       std::size_t p);

  virtual
  ~RelSatSolver();

  int
  solve(const TheoryPtr& theory, std::size_t sig_size);

  void
  solve();

  void
  receiveEOF();

  void
  receiveUNSAT(ClauseList& learned_clauses);

  void
  receiveSolution(DomainValue* _aAssignment, int _iVariableCount);

  void
  refresh();

  void
  print_local_theory();

private:
  bool
  prepare_input();

  void
  update_bridge_input(SignatureByCtx::const_iterator);

  void
  update_local_input(PartialBeliefSet& my_belief_set, 
		     SignatureByCtx::const_iterator low, 
		     SignatureByCtx::const_iterator up);

private:
  bool                           is_leaf;
  std::size_t                    my_id;
  std::size_t                    parent_session_id;
  std::size_t                    my_session_id;
  const TheoryPtr                theory;
  const SignaturePtr             sig;
  const BeliefStatePtr           localV;
  const HashedBiMap*             c2o;
  std::size_t                    system_size;      // this can be taken from localV
  MessagingGatewayBC*            mg;
  ConcurrentMessageQueue*        dmcs_sat_notif;   // to get notification from StreamingDMCS
  PartialBeliefState*            input;
  PartialBeliefStateBufPtr       input_buffer;
  bool                           first_round;

  SATInstance*                   xInstance;
  SATSolver*                     xSATSolver;

  std::size_t port;
};

typedef boost::shared_ptr<RelSatSolver> RelSatSolverPtr;

} // namespace dmcs

#endif // REL_SAT_SOLVER_H

// Local Variables:
// mode: C++
// End:
