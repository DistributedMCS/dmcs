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
 * @file   Cache.h
 * @author Thomas Krennwallner
 * @date   Thu Jun  1 11:03:43 2006
 * 
 * @brief  
 * 
 * 
 */


#ifndef _CACHE_H
#define _CACHE_H

#include "BeliefState.h"

#include <map>
#include <set>
#include <iosfwd>

#include <boost/ptr_container/indirect_fun.hpp>


namespace dmcs {

  ///@brief cache statistics
  class CacheStats
  {
  protected:
    unsigned m_hits;
    unsigned m_miss;
    unsigned m_bsno;
    unsigned m_total_bsno;

  public:
    /// default ctor
    CacheStats()
      : m_hits(0), m_miss(0), m_bsno(0), m_total_bsno(0)
    { }

    unsigned
    hits() const
    { return this->m_hits;  }

    void
    hits(int n)
    { this->m_hits += n; }

    unsigned
    miss() const
    { return this->m_miss; }

    void
    miss(int n)
    { this->m_miss += n;  }

    unsigned
    bsno() const
    { return this->m_bsno; }

    void
    bsno(int n)
    { this->m_bsno += n; }

    unsigned
    total_bsno() const
    { return this->m_total_bsno; }

    void
    total_bsno(int n)
    { this->m_total_bsno += n; }

  };

typedef boost::shared_ptr<CacheStats> CacheStatsPtr;


  /** 
   * Output the content of a CacheStats object @a cs.
   * 
   * @param os 
   * @param cs 
   * 
   * @return @a os
   */
  inline std::ostream&
  operator<< (std::ostream& os, const CacheStats& cs)
  {
    return os << "Cache hits: " << cs.hits() << std::endl
	      << "Cache miss: " << cs.miss() << std::endl
	      << "Number of cached belief states: " << cs.bsno() << std::endl
	      << "Total number of belief states: " << cs.total_bsno() << std::endl;
  }


  /**
   * @brief Base class for caching classes.
   */
  class BaseCache
  {
  protected:
    /// cache statistics
    mutable CacheStatsPtr stats;

  public:
    /// Ctor
    explicit
    BaseCache(CacheStatsPtr s)
      : stats(s)
    { }

    /// Dtor.
    virtual
    ~BaseCache()
    { }

    /**
     * checks if there is a cached entry for @a v.
     *
     * @param v
     *
     * @return 
     */
    virtual BeliefStateListPtr
    cacheHit(const BeliefStatePtr& v) const = 0;

    /** 
     */
    virtual void
    insert(const BeliefStatePtr& v, BeliefStateListPtr& s) = 0;

  };


  /** 
   * @brief The standard DMCS cache.
   *
   * @see 
   */
  class Cache : public BaseCache
  {
  protected:
    /// @brief we order the QueryCtx'en in the CacheSet accourding to
    /// their interpretations
    struct BeliefCmp
    {
      bool
      operator() (const BeliefStatePtr& v1, const BeliefStatePtr& v2) const
      {
    	return v1 < v2;
      }
    };

    /// cache
    //typedef std::set<BeliefState> CacheSet;

    /// maps V to BeliefStates
    typedef std::map<BeliefStatePtr, BeliefStateListPtr, BeliefCmp> QueryAnswerMap;
		     //		     boost::indirect_fun<std::less<BeliefState> > > QueryAnswerMap;

    /// the cache
    QueryAnswerMap cacheMap;

    // virtual BeliefStatesPtr
    // isValid(const BeliefState& v, const CacheSet& f) const;

  public:
    /// ctor
    explicit
    Cache(CacheStatsPtr s)
      : BaseCache(s),
	cacheMap()
    { }

    virtual BeliefStateListPtr
    cacheHit(const BeliefStatePtr& v) const;

    virtual void
    insert(const BeliefStatePtr& v, BeliefStateListPtr& s);

  };


typedef boost::shared_ptr<Cache> CachePtr;

  // /** 
  //  * For debugging purposes. 
  //  */
  // class DebugCache : public Cache
  // {
  // public:
  //   /// ctor
  //   explicit
  //   DebugCache(boost::shared_ptr<CacheStats> s)
  //     : Cache(s)
  //   { }

  //   virtual BeliefStatesPtr
  //   cacheHit(const BeliefStatePtr& v) const;

  // };


  // ///@brief does not cache anything at all.
  // class NullCache : public BaseCache
  // {
  // public:
  //   /// ctor
  //   explicit
  //   NullCache(boost::shared_ptr<CacheStats> s)
  //     : BaseCache(s)
  //   { }

  //   /**
  //    * @return an empty BeliefStatesPtr
  //    */
  //   BeliefStatesPtr
  //   cacheHit(const BeliefState& /* v */) const
  //   {
  //     return BeliefStatesPtr();
  //   }

  //   /** 
  //    * noop.
  //    */
  //   void
  //   insert(const BeliefState& /* v */, BeliefStatesPtr /* s */)
  //   { }
  // };


} // namespace dmcs

#endif /* _CACHE_H */


// Local Variables:
// mode: C++
// End:
