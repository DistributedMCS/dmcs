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
 * @file   Debug.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Thu Jan  6 10:41:06 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef _DMCS_DEBUG_H
#define _DMCS_DEBUG_H

#include "mcs/BeliefState.h"
#include "dmcs/QueryPlan.h"

#include <iosfwd>


namespace dmcs {

  void
  printBeliefStateNicely(std::ostream& os, const BeliefStatePtr& b_ptr, 
			 const BeliefStatePtr& V, const QueryPlanPtr& query_plan);

  void
  printBeliefStatesNicely(std::ostream& os, const BeliefStateListPtr& bs_ptr, 
			  const BeliefStatePtr& V, const QueryPlanPtr& query_plan);

} // namespace dmcs

#endif // _DMCS_DEBUG_H

// Local Variables:
// mode: C++
// End:
