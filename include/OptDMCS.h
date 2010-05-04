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
 * @file   OptDMCS.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Wed Nov  4 11:14:01 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef OPT_DMCS_H
#define OPT_DMCS_H

#include <boost/shared_ptr.hpp>

#include "BaseDMCS.h"
#include "OptMessage.h"
#include "ReturnMessage.h"
#include "Cache.h"
#include "Theory.h"

namespace dmcs {

class OptDMCS : public BaseDMCS
{
public:

#if defined(DMCS_STATS_INFO)
  typedef ReturnMessage      dmcs_value_type;
  typedef ReturnMessagePtr   dmcs_return_type;
#else
  typedef BeliefStateList    dmcs_value_type;
  typedef BeliefStateListPtr dmcs_return_type;
#endif

  OptDMCS(const ContextPtr& c, const TheoryPtr& t);

  virtual
  ~OptDMCS();

  dmcs_return_type
  getBeliefStates(const OptMessage& mess);

private:
  CacheStatsPtr cacheStats;
  CachePtr cache;
};

typedef boost::shared_ptr<OptDMCS> OptDMCSPtr;

} // namespace dmcs


#endif // OPT_DMCS_H

// Local Variables:
// mode: C++
// End:
