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
typedef boost::posix_time::time_duration TimeDuration;
typedef std::pair<TimeDuration, std::size_t> Info;
typedef std::map<std::size_t, Info> TransferTimes;
typedef boost::shared_ptr<TransferTimes> TransferTimesPtr;


#if defined(DMCS_STATS_INFO)
#define STATS_DIFF(a,b)						   \
  TimeDuration b(0,0,0,0);			   \
  {								   \
    PTime begin = boost::posix_time::microsec_clock::local_time(); \
    a;								   \
    PTime end = boost::posix_time::microsec_clock::local_time();   \
    b = end - begin;						   \
  }

#define STATS_DIFF_REUSE(a,b)					   \
  {								   \
    PTime begin = boost::posix_time::microsec_clock::local_time(); \
    a;								   \
    PTime end = boost::posix_time::microsec_clock::local_time();   \
    b += end - begin;						   \
  }
#else
#define STATS_DIFF(a,b) do { a; } while(0);
#define STATS_DIFF_REUSE(a,b) do { a; } while(0);
#endif // DMCS_STATS_INFO


namespace dmcs {

  /*
inline TimeDuration&
getTimeDuration(Info& info)
{
  return info.first;
  }*/

inline TimeDuration
getTimeDuration(const Info& info)
{
  return info.first;
}


inline std::size_t
getNoModels(Info& info)
{
  return info.second;
}


inline std::ostream&
operator<< (std::ostream& os, const Info& info)
{
  os << boost::posix_time::to_simple_string( getTimeDuration(info) )
     << ", "
     << info.second;

  return os;
}

struct StatsInfo
{
  Info lsolve;
  Info combine;
  Info projection;
  TransferTimesPtr transfer;

  StatsInfo()
  { }

  StatsInfo(Info lsolve_, 
	    Info combine_, 
	    Info projection_, 
	    TransferTimesPtr transfer_)
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

typedef std::vector<StatsInfo> StatsInfos;
typedef boost::shared_ptr<StatsInfos> StatsInfosPtr;



inline std::ostream&
operator<< (std::ostream& os, const StatsInfo& si)
{
  os << si.lsolve << ", " << si.projection << ", " << si.combine;

  int i = -1;

  if (si.transfer->size() > 0)
    {
      os << ", ";
      TransferTimes::const_iterator end = --si.transfer->end();
      for (TransferTimes::const_iterator it = si.transfer->begin(); it != end; ++it, --i)
	{
	  os << i << ", " << it->first << ", " << it->second << ", ";
	}
      os << i << ", " << end->first << ", " << end->second;
    }

  
  return os;
}



inline std::ostream&
operator<< (std::ostream& os, const StatsInfos& sis)
{
  //std::copy(sis.begin(), sis.end(), std::ostream_iterator<StatsInfo>(os, "\n");
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
      TransferTimesPtr transfer1       = it1->transfer;
      const TransferTimesPtr transfer2 = it2->transfer;

       ///@todo: use helper function getTimeDuration
      it1->lsolve.first     += it2->lsolve.first;
      it1->combine.first    += it2->combine.first;
      it1->projection.first += it2->projection.first;

      if (it1->lsolve.second == 0)
	{
	  it1->lsolve.second = it2->lsolve.second;
	}

      if (it1->combine.second == 0)
	{
	  it1->combine.second = it2->combine.second;
	}

      if (it1->projection.second == 0)
	{
	  it1->projection.second = it2->projection.second;
	}

      if ((transfer1->size() == 0) && (transfer2->size() > 0))
	{
	  *transfer1 = *transfer2; // copy here
	}
    }
}

} // namespace dmcs

#endif // STATS_INFO_H

// Local Variables:
// mode: C++
// End:
