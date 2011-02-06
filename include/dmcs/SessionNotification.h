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
 * @file   SessionNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Feb  5 20:38:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef SESSION_NOTIFICATION_H
#define SESSION_NOTIFICATION_H

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcs {

struct SessionNotification
{
  enum NotificationType
    {
      REQUEST = 0,
      SHUTDOWN
    };

  SessionNotification(std::size_t sid, NotificationType t = REQUEST)
    : session_id(sid),
      type(t)
  { }
  
  std::size_t session_id;
  NotificationType type;
};

inline std::ostream&
operator<< (std::ostream& os, const SessionNotification& sn)
{
  os << "("<< sn.session_id << ")";
  return os;
}

} // namespace dmcs

#endif // SESSION_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
