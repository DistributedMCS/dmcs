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
 * @file   NoSBAQualitySortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jul 14  9:17:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef NO_SBA_QUALITY_CTX_SORTING_STRATEGY_H
#define NO_SBA_QUALITY_CTX_SORTING_STRATEGY_H

#include "ContextSortingStrategy.h"
#include "Match.h"

namespace dmcs {

class NoSBAQualityCtxSortingStrategy : public ContextSortingStrategy
{
public:
  // IteratorListPtr == shared_ptr of list of iterators of a MatchTableIteratorVec
  NoSBAQualityCtxSortingStrategy(IteratorListPtr list_to_sort_, CountVecPtr sba_count_,
				 std::size_t dfs_level_)
    : ContextSortingStrategy(list_to_sort_, dfs_level_),
      sba_count(sba_count_)
  { }

  void calculateQuality()
  {

#ifdef DEBUG
    std::cerr << TABS(dfs_level) << "Calculating qualities (NoSBAQualitySortingStrategy):" << std::endl;
#endif

    for (IteratorList::const_iterator it = list_to_sort->begin(); it != list_to_sort->end(); ++it)
      {
	// the quality takes the number of schematic bridge atoms in a
	// potential neighbor into account. Furthermore, the quality
	// of the match also counts. With this setting, can sort
	// according to 2 criteria in one shot: 

	// + Firstly, sort contexts wrt. their numbers of schematic
	// bridge atoms, increasingly

	// + Then, the contexts with the same numbers of schematic
	// bridge atoms are sorted decreasingly according to the
	// quality of the respective match. Note that the quality of a
	// match is in [0,1]

	ContextID cid = (**it)->tarCtx;
	float mq = (**it)->quality;
	float q_it =  (*sba_count)[cid] - mq;

#ifdef DEBUG
	std::cerr << TABS(dfs_level) << cid << ", " << q_it << std::endl;
#endif

	quality->insert( std::pair<MatchTableIteratorVec::iterator, float>(*it, q_it) );
      }
  }

private:
  CountVecPtr sba_count;
};

} // namespace dmcs

#endif // NO_SBA_QUALITY_SORTING_STRATEGY_H

// Local Variables:
// mode: C++
// End:
