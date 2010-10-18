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
 * @file   ChosenQualityCtxSortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jul 17 09:07:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef CHOSEN_QUALITY_CTX_SORTING_STRATEGY_H
#define CHOSEN_QUALITY_CTX_SORTING_STRATEGY_H

#include "ContextSubstitution.h"
#include "ContextSortingStrategy.h"
#include "Match.h"

namespace dmcs {

class ChosenQualityCtxSortingStrategy : public ContextSortingStrategy
{
public:
  ChosenQualityCtxSortingStrategy(IteratorListPtr list_to_sort_, 
				  ContextSubstitutionPtr ctx_substitution_sofar_,
				  std::size_t pool_size_,
				  std::size_t dfs_level_)
    : ContextSortingStrategy(list_to_sort_, dfs_level_),
      ctx_substitution_sofar(ctx_substitution_sofar_),
      pool_size(pool_size_)
  { }

  void calculateQuality()
  {

#ifdef DEBUG
    std::cerr << TABS(dfs_level) << "Calculating qualities (ChosenQualityCtxSortingStrategy):" << std::endl;
    std::cerr << TABS(dfs_level) << "" << std::endl;
#endif

    for (IteratorList::const_iterator it = list_to_sort->begin(); it != list_to_sort->end(); ++it)
      {
	ContextID cid = (**it)->tarCtx;
	float mq = (**it)->quality;
	
	// try a very simple quality measurement: if the context was
	// chosen, then give it a high priority than the unchosen
	// ones. Among the chosen one, choose the one with highest
	// quality of the match

	float q_cid;
	if (already_chosen(ctx_substitution_sofar, cid))
	  {
	    q_cid = -mq;
	  }
	else
	  {
	    q_cid = 2 - mq;
	  }

#ifdef DEBUG
	std::cerr << TABS(dfs_level) << cid << ", " << q_cid << std::endl;
#endif

	quality->insert( std::pair<MatchTableIteratorVec::iterator, float>(*it, q_cid) );
      }
  }

private:
  ContextSubstitutionPtr ctx_substitution_sofar;
  std::size_t pool_size;
};

} // namespace dmcs

#endif // CHOSEN_QUALITY_CTX_SORTING_STRATEGY_H

// Local Variables:
// mode: C++
// End:
