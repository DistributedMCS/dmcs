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
 * @file   NoSBACtxSortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jul 13  15:42:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef NO_SBA_NEIGHBOR_SORTING_STRATEGY
#define NO_SBA_NEIGHBOR_SORTING_STRATEGY

#include "BaseSortingStrategy.h"
#include "ContextSubstitution.h"

namespace dmcs {

///@todo: may be there is a way to unify NoSBANeighborSortingStrategy
/// and NoSBACtxSortingStrategy with template
class NoSBANeighborSortingStrategy : public BaseSortingStrategy<ContextVec::iterator>
{
public:
  NoSBANeighborSortingStrategy(IteratorListPtr list_to_sort_, 
			       CountVecPtr sba_count_,
			       std::size_t dfs_level_)
    : BaseSortingStrategy<ContextVec::iterator>(list_to_sort_, dfs_level_),
      sba_count(sba_count_)
  { }

  void
  calculateQuality()
  {
#ifdef DEBUG
    std::cerr << TABS(dfs_level) << "Calculating qualities:" << std::endl;
#endif

    for (IteratorList::const_iterator it = list_to_sort->begin(); it != list_to_sort->end(); ++it)
      {
	ContextID cid = **it;
	float q_it =  (*sba_count)[cid];

#ifdef DEBUG
	std::cerr << TABS(dfs_level) << cid << ", " << q_it << std::endl;
#endif

	quality->insert( std::pair<ContextVec::iterator, float>(*it, q_it) );
      }
  }

private:
  CountVecPtr sba_count;
};

}

#endif // NO_SBA_NEIGHBOR_SORTING_STRATEGY

// Local Variables:
// mode: C++
// End:

