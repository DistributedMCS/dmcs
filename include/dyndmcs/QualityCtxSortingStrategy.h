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
 * @file   QualityCtxSortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Oct 13  9:17:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef QUALITY_CTX_SORTING_STRATEGY_H
#define QUALITY_CTX_SORTING_STRATEGY_H

#include "ContextSortingStrategy.h"
#include "Match.h"

namespace dmcs {

class QualityCtxSortingStrategy : public ContextSortingStrategy
{
public:
  // IteratorListPtr == shared_ptr of list of iterators of a MatchTableIteratorVec
  QualityCtxSortingStrategy(IteratorListPtr list_to_sort_, std::size_t dfs_level_)
    : ContextSortingStrategy(list_to_sort_, dfs_level_)
  { }

  void calculateQuality()
  {

#ifdef DEBUG
    std::cerr << TABS(dfs_level) << "Calculating qualities:" << std::endl;
#endif

    for (IteratorList::const_iterator it = list_to_sort->begin(); it != list_to_sort->end(); ++it)
      {
	// this quality directly takes the quality of the match into
	// account.
	float q_it =  -(**it)->quality;

#ifdef DEBUG
	ContextID cid = (**it)->tarCtx;
	std::cerr << TABS(dfs_level) << cid << ", " << q_it << std::endl;
#endif

	quality->insert( std::pair<MatchTableIteratorVec::iterator, float>(*it, q_it) );
      }
  }
};

} // namespace dmcs

#endif // QUALITY_CTX_SORTING_STRATEGY_H

// Local Variables:
// mode: C++
// End:
