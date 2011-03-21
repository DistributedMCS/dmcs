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
 * @file   StreamingDMCSNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  13 7:10:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_DMCS_NOTIFICATION_H
#define STREAMING_DMCS_NOTIFICATION_H

#include "dmcs/BaseNotification.h"
#include "dmcs/BaseTypes.h"

namespace dmcs {



struct StreamingDMCSNotification : public BaseNotification
{
  StreamingDMCSNotification(NotificationType t, 
#ifdef DEBUG
			    History pa,
#else
			    std::size_t pa,
#endif
			    std::size_t i,
			    std::size_t sid, 
			    std::size_t ps,
			    std::size_t p)
    : BaseNotification(t, pa),
      invoker(i),
      session_id(sid),
      pack_size(ps),
      port(p)
  { }

  std::size_t invoker;
  std::size_t session_id;
  std::size_t pack_size;
  std::size_t port;
};



inline std::ostream&
operator<< (std::ostream& os, const StreamingDMCSNotification& sdn)
{
  os << (BaseNotification)sdn
     << " invoker = " << sdn.invoker
     << " sid = " << sdn.session_id 
     << " pack_size = " << sdn.pack_size
     << " port = " << sdn.port;

  return os;
}



} // namespace dmcs

#endif // STREAMING_DMCS_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
