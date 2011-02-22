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
 * @file   BaseSortingStrategy.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jun 28  15:24:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_SORTING_STRATEGY
#define BASE_SORTING_STRATEGY

#include <map>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <iomanip>

#include "ContextSubstitution.h"

namespace dmcs {

#define NO_SBA_CTX_SORTING_STRATEGY 1
#define NO_SBA_QUALITY_CTX_SORTING_STRATEGY 3
#define CHOSEN_CTX_SORTING_STRATEGY 2
#define CHOSEN_QUALITY_CTX_SORTING_STRATEGY 4
#define QUALITY_CTX_SORTING_STRATEGY 5

// Macro for writing out n tabs, which is used for readable debugging
// output, especially for any DFS algorithm.
// This macro can be put at the global scope if needed.
#define TAB 3
#define TABS(n)								\
  std::setw(TAB) << std::setfill('-') << n << std::setw(n * TAB) << std::setfill(' ') << ' ' \

// The idea is to have a generic sorting strategy for a given
// vector of objects, each element of the vector is associated
// to a quality value. How the quality is determined depends on the
// specific sorting strategy, which is a subclass of this.

// The problem is how to relate the elements in the vector to their
// quality values. To make it generic, we collect the iterators of the
// elements and store them in a list, and then map each position in
// the vector to a quality value. Note that using vector is mandatory
// because a map needs its keys comparable with operator<.

// Then we sort the list of iterator according to the quality of the
// real elements, and later we can use 

template<typename IteratorType>
class BaseSortingStrategy;


template<typename IteratorType>
class CompareQuality
{
public:
  CompareQuality(typename BaseSortingStrategy<IteratorType>::QualityMapPtr quality_,
		 std::size_t /* dfs_level_ */ = 0)
    : quality(quality_)
  { }
  
  bool operator() (IteratorType i, IteratorType j)
  {
    typename BaseSortingStrategy<IteratorType>::QualityMap::const_iterator ii;
    typename BaseSortingStrategy<IteratorType>::QualityMap::const_iterator ij;

    ii = quality->find(i);    
    ij = quality->find(j);
    
    float qi = ii->second;
    float qj = ij->second;
    
    return (qi < qj);
  }
  
private:
  typename BaseSortingStrategy<IteratorType>::QualityMapPtr quality;
};
  

template<typename IteratorType>
class BaseSortingStrategy
{
public:
  typedef std::list<IteratorType> IteratorList;
  typedef boost::shared_ptr<IteratorList> IteratorListPtr;
  typedef std::pair<IteratorType, float> Quality;
  typedef std::map<IteratorType, float> QualityMap;
  typedef boost::shared_ptr<QualityMap> QualityMapPtr;

  BaseSortingStrategy(IteratorListPtr list_to_sort_, std::size_t dfs_level_)
    : list_to_sort(list_to_sort_),
      dfs_level(dfs_level_),
      quality(new QualityMap)
  { }

  virtual void 
  calculateQuality() = 0;

  void
  sort()
  {
    calculateQuality();
    list_to_sort->sort(CompareQuality<IteratorType>(quality));
  }

protected:
  IteratorListPtr list_to_sort;
  std::size_t dfs_level;
  QualityMapPtr quality;
};

} // namespace dmcs

#endif

// Local Variables:
// mode: C++
// End:
