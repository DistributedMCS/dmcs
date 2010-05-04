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
 * @file   StatsInfo.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun May  02 18:30:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef STATS_INFO_H
#define STATS_INFO_H

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

typedef boost::posix_time::ptime PTime;
typedef boost::shared_ptr<PTime> PTimePtr;
typedef boost::posix_time::time_duration TimeDuration;
typedef boost::shared_ptr<TimeDuration> TimeDurationPtr;
typedef std::map<std::size_t, TimeDurationPtr> TransferTimes;
typedef boost::shared_ptr<TransferTimes> TransferTimesPtr;


#if defined(DMCS_STATS_INFO)
#define STATS_DIFF(a,b)						   \
  TimeDurationPtr b(new TimeDuration(0,0,0,0));			   \
  {								   \
    PTime begin = boost::posix_time::microsec_clock::local_time(); \
    a;								   \
    PTime end = boost::posix_time::microsec_clock::local_time();   \
    *b = end - begin;						   \
  }

#define STATS_DIFF_REUSE(a,b)					   \
  {								   \
    PTime begin = boost::posix_time::microsec_clock::local_time(); \
    a;								   \
    PTime end = boost::posix_time::microsec_clock::local_time();   \
    *b += end - begin;						   \
  }
#else
#define STATS_DIFF(a,b) do { a; } while(0);
#define STATS_DIFF_REUSE(a,b) do { a; } while(0);
#endif // DMCS_STATS_INFO


namespace dmcs {

struct StatsInfo
{
  TimeDurationPtr lsolve;
  TimeDurationPtr combine;
  TimeDurationPtr projection;
  TransferTimesPtr transfer;

  StatsInfo()
  { }

  StatsInfo(TimeDurationPtr& lsolve_, 
	    TimeDurationPtr& combine_, 
	    TimeDurationPtr& projection_, 
	    TransferTimesPtr& transfer_)
    : lsolve(lsolve_), 
      combine(combine_), 
      projection(projection_), 
      transfer(transfer_)
  { }

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* version */)
  {
    ar & lsolve;
    ar & combine;
    ar & projection;
    ar & transfer;
  }
};

typedef boost::shared_ptr<StatsInfo> StatsInfoPtr;
typedef std::vector<StatsInfoPtr> StatsInfos;
typedef boost::shared_ptr<StatsInfos> StatsInfosPtr;



inline std::ostream&
operator<< (std::ostream& os, const StatsInfo& si)
{
  os << "[" << boost::posix_time::to_simple_string(*si.lsolve) << "], "
     << "[" << boost::posix_time::to_simple_string(*si.combine) << "], " 
     << "[" << boost::posix_time::to_simple_string(*si.projection) << "]" 
     << std::endl;

  os << "Transfer size = " << si.transfer->size() << std::endl;

  if (!si.transfer->empty())
    {
      TransferTimes::const_iterator end = --si.transfer->end();
      
      for (TransferTimes::const_iterator it = si.transfer->begin(); it != end; ++it)
	{
	  os << "(" << it->first << ", " << boost::posix_time::to_simple_string(*it->second) << "); ";
	}
      os << "(" << end->first << ", " << boost::posix_time::to_simple_string(*end->second) << ")";
    }
  
  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const StatsInfoPtr& si)
{
  return os << *si;
}


inline std::ostream&
operator<< (std::ostream& os, const StatsInfos& sis)
{
  for (StatsInfos::const_iterator it = sis.begin(); it != sis.end(); ++it)
    {
      os << *it << std::endl;
    }

  return os;
}


inline void
combine(StatsInfosPtr& sis1, const StatsInfosPtr& sis2)
{
  assert(sis1->size() == sis2->size());

  StatsInfos::iterator it1 = sis1->begin();
  StatsInfos::const_iterator it2 = sis2->begin();

  for (; it1 != sis1->end(); ++it1, ++it2)
    {
      TimeDurationPtr  lsolve1     = (*it1)->lsolve;
      TimeDurationPtr  combine1    = (*it1)->combine;
      TimeDurationPtr  projection1 = (*it1)->projection;
      TransferTimesPtr transfer1   = (*it1)->transfer;

      const TimeDurationPtr  lsolve2     = (*it2)->lsolve;
      const TimeDurationPtr  combine2    = (*it2)->combine;
      const TimeDurationPtr  projection2 = (*it2)->projection;
      const TransferTimesPtr transfer2   = (*it2)->transfer;

      *lsolve1     = *lsolve1     + (*lsolve2);
      *combine1    = *combine1    + (*combine2);
      *projection1 = *projection1 + (*projection2);

      if ((transfer1->size() == 0) && (transfer2->size() > 0))
	{
	  *transfer1   = *transfer2; // copy here
	}
    }
}

} // namespace dmcs

#endif // STATS_INFO_H

// Local Variables:
// mode: C++
// End:
