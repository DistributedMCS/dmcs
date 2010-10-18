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
 * @file   RedBBodySortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jul  5  16:57:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef REDUCED_BRIDGE_BODY_SORTING_STRATEGY
#define REDUCED_BRIDGE_BODY_SORTING_STRATEGY

#include "BaseSortingStrategy.h"

namespace dmcs {

class RedBBodySortingStrategy : public BaseSortingStrategy<ReducedBridgeBody::iterator>
{
public:
  RedBBodySortingStrategy(IteratorListPtr list_to_sort_, std::size_t dfs_level_)
    : BaseSortingStrategy<ReducedBridgeBody::iterator>(list_to_sort_, dfs_level_)
  { }

  virtual void
  calculateQuality() = 0;
};

} // namespace dmcs

#endif // REDUCED_BRIDGE_BODY_SORTING_STRATEGY

