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
 * @file   NoSBARedBBodySortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jul  5  16:57:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef NO_SBA_REDUCED_BRIDGE_BODY_SORTING_STRATEGY
#define NO_SBA_REDUCED_BRIDGE_BODY_SORTING_STRATEGY

#include "RedBBodySortingStrategy.h"

namespace dmcs {

class NoSBARedBBodySortingStrategy : public RedBBodySortingStrategy
{
public:
  NoSBARedBBodySortingStrategy(IteratorListPtr list_to_sort_, CountVecPtr sba_count_,
			       MatchTablePtr mt_, std::size_t dfs_level_)
    : RedBBodySortingStrategy(list_to_sort_, dfs_level_),
      sba_count(sba_count_),
      mt(mt_)
  { }

  // The quality for a schematic bridge atom (X:[p]) is calculated as
  // follows: find all contexts that [p] has matches to (from
  // MatchTable). Each such context has its number of sbridge atoms,
  // choose the minimal value and assign it to the quality of (X:[p])
  void
  calculateQuality()
  {
    const MatchTableBySrcSym& ss = boost::get<Tag::SrcSym>(*mt);

    for (IteratorList::iterator it = list_to_sort->begin();
	 it != list_to_sort->end(); ++it)
      {
	BridgeAtom sba = **it;

	ContextTerm ctt = sba.first;
	SchematicBelief sb = sba.second;

	assert(isCtxVar(ctt));

#ifdef DEBUG
	std::cerr << TABS(dfs_level) << "Calculating quality for sbridge atom " 
		  << ctx2string(ctt) << std::endl;
#endif

	ContextID context_id = ctxID(ctt);
	std::size_t s_const = sBelief(sb);

	MatchTableBySrcSym::const_iterator low = ss.lower_bound(boost::make_tuple(context_id, s_const));
	MatchTableBySrcSym::const_iterator up  = ss.upper_bound(boost::make_tuple(context_id, s_const));

	// make it simple like this now. Later we can just drop
	// sbridge atoms with no match
	assert(std::distance(low, up) != 0);

	float min = std::numeric_limits<float>::max();
	for (; low != up; ++low)
	  {
	    ContextID potential_neighbor_id = low->tarCtx;

	    float q = (*sba_count)[potential_neighbor_id];
	    
	    if (min > q)
	      {
		min = q;
	      }
#ifdef DEBUG
	    std::cerr << TABS(dfs_level) << "Checking potential neighbor " 
		      << potential_neighbor_id << ", min = " << min << std::endl;
#endif

	  }

	quality->insert( std::pair<ReducedBridgeBody::iterator, float>(*it, min) );
      }
  }

private:
  CountVecPtr sba_count;
  MatchTablePtr mt;
};

} // namespace dmcs

#endif // NO_SBA_REDUCED_BRIDGE_BODY_SORTING_STRATEGY

// Local Variables:
// mode: C++
// End:
