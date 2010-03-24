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
 * @file   BeliefCombination.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Nov  17 16:53:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef BELIEF_COMBINATION_H
#define BELIEF_COMBINATION_H

#include "BeliefState.h"
#include "QueryPlan.h"
#include "Theory.h"

#include <iostream>


namespace dmcs {

void
printBeliefStateNicely(std::ostream& os, const BeliefStatePtr& b_ptr, 
		       const BeliefStatePtr& V, const QueryPlanPtr& query_plan);

void
printBeliefStatesNicely(std::ostream& os, const BeliefStatesPtr& bs_ptr, 
			const BeliefStatePtr& V, const QueryPlanPtr& query_plan);

void
update(BeliefStatePtr& s, const BeliefStatePtr& t);

bool
combine(const BeliefStatePtr& s, const BeliefStatePtr& t, BeliefStatePtr& u, const QueryPlanPtr& query_plan);

BeliefStatesPtr 
combine(const BeliefStatesPtr& cs, const BeliefStatesPtr& ct, const QueryPlanPtr& query_plan);

void
project_to(const BeliefStatesPtr& cs, const BeliefStatePtr& v, BeliefStatesPtr& cu);

} // namespace dmcs

#endif // BELIEF_COMBINATION_H

// Local Variables:
// mode: C++
// End:
