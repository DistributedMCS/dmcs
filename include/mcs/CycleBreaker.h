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
 * @file   CycleBreaker.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  11 18:15:26 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef __CYCLE_BREAKER_H__
#define __CYCLE_BREAKER_H__

#include <boost/shared_ptr.hpp>

#include "dmcs/Evaluator.h"
#include "mcs/BridgeRuleEvaluator.h"
#include "mcs/QueryPlan.h"
#include "mcs/RequestDispatcher.h"
#include "mcs/StreamingJoiner.h"
#include "network/NewConcurrentMessageDispatcher.h"
#include "mcs/NewContext.h"

namespace dmcs {

class CycleBreaker : public NewContext
{
public:
  CycleBreaker(std::size_t cid,
	       EvaluatorPtr eval,
	       ReturnPlanMapPtr return_plan,
	       ContextQueryPlanMapPtr queryplan_map,
	       BridgeRuleTablePtr bridge_rules);

  ~CycleBreaker();

  void
  startup(NewConcurrentMessageDispatcherPtr md,
	  RequestDispatcherPtr rd,
	  NewJoinerDispatcherPtr jd);

private:
  void
  init();

private:
  std::size_t breaker_offset;
  EvaluatorPtr eval;
};


typedef boost::shared_ptr<CycleBreaker> CycleBreakerPtr;

struct CycleBreakerWrapper
{
  void
  operator()(CycleBreakerPtr cycle_breaker,
	     NewConcurrentMessageDispatcherPtr md,
	     RequestDispatcherPtr rd,
	     NewJoinerDispatcherPtr jd)
  {
    cycle_breaker->startup(md, rd, jd);
  }
};

} // namespace dmcs

#endif // __CYCLE_BREAKER_H__

// Local Variables:
// mode: C++
// End:
