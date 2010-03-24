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
 * @file   QueryPlanGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  20 23:12:00 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef QUERY_PLAN_GENERATOR_H
#define QUERY_PLAN_GENERATOR_H

#include "BeliefState.h"
#include "Context.h"
#include "QueryPlan.h"


namespace dmcs { namespace generator {

class QueryPlanGenerator
{
public:
  QueryPlanGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_);

  void generate();

  virtual void
  genNeighbors(const ContextPtr& context) = 0;

  void
  genNeighbor(const ContextPtr& context, std::size_t id);

  void create_interfaces();

  virtual void
  create_interface(const ContextPtr& context) = 0;

  const BeliefStatePtr&
  getMinV() const;

protected:
  int 
  local_interface(std::size_t context_id1, std::size_t context_id2);

  void
  initialize_local_interface();

  void
  compute_min_V();

protected:
  ContextsPtr& contexts;
  QueryPlanPtr& query_plan;
  BeliefStatePtr V;
};


  } // namespace generator
} // namespace dmcs

#endif // QUERY_PLAN_GENERATOR_H

// Local Variables:
// mode: C++
// End:

