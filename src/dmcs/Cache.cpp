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
 * @file   Cache.cpp
 * @author Thomas Krennwallner
 * @date   Mon Jul  3 11:10:38 2006
 * 
 * @brief  
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "dmcs/Cache.h"

#include "BeliefState.h"
#include "BeliefCombination.h"

#include <iterator>
#include <functional>
#include <iostream>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/shared_ptr.hpp>

using namespace dmcs;


// BeliefStatesPtr
// Cache::isValid(const BeliefState& query, const Belief& found) const
// {
  // for (CacheSet::const_iterator it = found.begin(); it != found.end(); ++it)
  //   {
  //     const Query& q1 = query->getQuery();
  //     const Query& q2 = (*it)->getQuery();

  //     const AtomSet& i = q1.getProjectedInterpretation();
  //     const AtomSet& j = q2.getProjectedInterpretation();

  //     ///@todo support for cq missing

  //     if (q1.getDLQuery()->isBoolean())
  // 	{
  // 	  bool isPositive = (*it)->getAnswer().getAnswer();
	  
  // 	  if (isPositive)
  // 	    {
  // 	      // does i include j, i.e. is j \subseteq i?
  // 	      if (std::includes(i.begin(), i.end(), j.begin(), j.end()))
  // 		{
  // 		  return *it;
  // 		}
  // 	    }
  // 	  else
  // 	    {
  // 	      // does j include i, i.e. is j \supseteq i?
  // 	      if (std::includes(j.begin(), j.end(), i.begin(), i.end()))
  // 		{
  // 		  return *it;
  // 		}
  // 	    }
  // 	}
  //     else // retrieval modes
  // 	{
  // 	  // is the set of ints in query equal to the set of ints in found?
  // 	  if (i == j)
  // 	    {
  // 	      return *it;
  // 	    }
  // 	}
  //   }

  // return QueryCtx::shared_pointer(); // nothing found
//}


BeliefStateListPtr
Cache::cacheHit(const BeliefStatePtr& v) const
{
  // find if there exists any cache (v*, s*) such that v < v*
  //std::cerr << "Looking into cache..." << std::endl;
  for (QueryAnswerMap::const_reverse_iterator i = cacheMap.rbegin(); i != cacheMap.rend(); ++i)
    {
      if (v <= i->first)
	{
	  stats->hits(1);
	  //std::cerr << "In cacheHit: Found cached information..." << std::endl << *stats << std::endl;
	  return i->second;
	}
    }      

  stats->miss(1);
  //std::cerr << "In cacheHit: Found nothing..." << std::endl << *stats << std::endl;
  return BeliefStateListPtr();
}


// namespace dlvhex {
//   namespace dl {

//     struct InterSubset : public std::unary_function<const QueryCtx::shared_pointer&, bool>
//     {
//       const QueryCtx::shared_pointer& q1;

//       InterSubset(const QueryCtx::shared_pointer& q)
// 	: q1(q)
//       { }

//       bool
//       operator() (const QueryCtx::shared_pointer& q2) const
//       {
// 	const AtomSet& i = q1->getQuery().getProjectedInterpretation();
// 	const AtomSet& j = q2->getQuery().getProjectedInterpretation();
// 	// does i include j and i != j, i.e. is j \subset i?
// 	return i != j && std::includes(i.begin(), i.end(), j.begin(), j.end());
//       }
//     };

//     struct InterSuperset : public std::unary_function<const QueryCtx::shared_pointer&, bool>
//     {
//       const QueryCtx::shared_pointer& q1;

//       InterSuperset(const QueryCtx::shared_pointer& q)
// 	: q1(q)
//       { }

//       bool
//       operator() (const QueryCtx::shared_pointer& q2) const
//       {
// 	const AtomSet& i = q1->getQuery().getProjectedInterpretation();
// 	const AtomSet& j = q2->getQuery().getProjectedInterpretation();
// 	// does j include i and i != j, i.e. is j \supset i?
// 	return i != j && std::includes(j.begin(), j.end(), i.begin(), i.end());
//       }
//     };

//   } // namespace dl
// } // namespace dlvhex


void
Cache::insert(const BeliefStatePtr& v, BeliefStateListPtr& s)
{
  QueryAnswerMap::iterator foundit = cacheMap.find(v);
  if (foundit == cacheMap.end())
    {
      //std::cerr << "Insert to cache" << std::endl;
      // remove all pairs (v*, s) such that v* < v
      for (QueryAnswerMap::iterator i = cacheMap.begin(); i != cacheMap.end(); ++i)
	{
	  //std::cerr << "Remove smaller cache" << std::endl;
	  if (i->first < v)
	    {
	      cacheMap.erase(i);
	      --i;
	    }
	}
      
      //std::cerr << "Insert to cache " << v;
      QueryAnswerMap::value_type p(v, s);
      cacheMap.insert(p);
      stats->bsno(1);
    }
}


// BeliefStatesPtr
// DebugCache::cacheHit(const BeliefState& v) const
// {
  // std::cerr << "===== now looking for dl-query a = " << query->getQuery() << std::endl;

  // std::cerr << "----- cache content:" << std::endl;

  // for (QueryAnswerMap::const_iterator it = cacheMap.begin();
  //      it != cacheMap.end(); ++it)
  //   {
  //     std::cerr << *(it->first) << std::endl;
  //   }

  // const CacheSet* found = Cache::find(query);

  // if (found)
  //   {
  //     std::cerr << "----- found cache(a): " << std::endl;

  //     for (CacheSet::const_iterator it = found->begin();
  // 	   it != found->end(); ++it)
  // 	{
  // 	  std::cerr << *(*it) << " = " << (*it)->getAnswer() << std::endl;
  // 	}

  //     QueryCtx::shared_pointer p = Cache::isValid(query, *found);

  //     if (p)
  // 	{
  // 	  std::cerr << "===== cache-hit for a is " << *p << std::endl;
  // 	  stats.hits(1);
  // 	  return p;
  // 	}
  //     else
  // 	{
  // 	  std::cerr << "===== NOT a cache-hit" << std::endl;
  // 	}
  //   }
  // else
  //   {
  //     std::cerr << "===== NOT found in cache" << std::endl;
  //   }

  // stats.miss(1);

  // return QueryCtx::shared_pointer();
// }


// Local Variables:
// mode: C++
// End:
