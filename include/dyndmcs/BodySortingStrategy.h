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
 * @file   BodySortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jul  4  17:05:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef BODY_SORTING_STRATEGY
#define BODY_SORTING_STRATEGY

#include "BaseSortingStrategy.h"

namespace dmcs {

class BodySortingStrategy : public BaseSortingStrategy<BridgeAtomListPtr, ContextID>
{
public:
  ContextSortingStrategy(ContextListPtr context_list_)
    : BaseSortingStrategy<ContextListPtr, ContextID>(context_list_)
  { }

  virtual void
  calculateQuality() = 0;
};

}

#endif // BODY_SORTING_STRATEGY

// Local Variables:
// mode: C++
// End:

