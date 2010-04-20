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
 * @file   PrimitiveDMCS.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Wed Nov  4 11:14:01 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef OPT_DMCS_H
#define OPT_DMCS_H

#include "BaseDMCS.h"
#include "Message.h"
#include "Cache.h"
#include "Theory.h"

namespace dmcs {

class OptDMCS : public BaseDMCS
{
public:
  OptDMCS(ContextPtr& c, TheoryPtr& t);

  virtual
  ~OptDMCS();

  BeliefStatesPtr
  getBeliefStates(OptMessage& mess);

private:
  void
  localSolve(const BeliefStatePtr& V);

private:
  BeliefStatesPtr belief_states;
  BeliefStatesPtr local_belief_states;
  CacheStatsPtr cacheStats;
  CachePtr cache;
  TheoryPtr theory;
};


} // namespace dmcs

#include "OptDMCS.tcc"

#endif // OPT_DMCS_H

// Local Variables:
// mode: C++
// End:
