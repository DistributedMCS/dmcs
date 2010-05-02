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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/map.hpp>

typedef boost::posix_time::time_duration TimeDuration;
typedef std::map<std::size_t, TimeDuration> TransferTime;

namespace dmcs {

struct StatsInfo
{
  TimeDuration lsolve;
  TimeDuration join;
  TimeDuration projection;
  TransferTime transfer;

  StatsInfo(TimeDuration& lsolve_, TimeDuration& join_, TimeDuration& projection_, TransferTime& transfer_)
  : lsolve(lsolve_), join(join_), projection(projection_), transfer(transfer_)
  { }

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* version */)
  {
    ar & lsolve;
    ar & join;
    ar & projection;
    ar & transfer;
  }
};

typedef boost::shared_ptr<StatsInfo> StatsInfoPtr;

} // namespace dmcs

#endif // STATS_INFO_H

// Local Variables:
// mode: C++
// End:
