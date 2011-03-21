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
 * @file   OutputNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  12 8:43:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef OUTPUT_NOTIFICATION_H
#define OUTPUT_NOTIFICATION_H

#include "dmcs/BaseNotification.h"

namespace dmcs {



struct OutputNotification : public BaseNotification
{
  OutputNotification(NotificationType t, 
#ifdef DEBUG
		     History pa,
#else
		     std::size_t pa,
#endif
		     std::size_t psid, 
		     std::size_t ps)
    : BaseNotification(t, pa),
      parent_session_id(psid),
      pack_size(ps)
  { }
  
  std::size_t parent_session_id;
  std::size_t pack_size;
};



inline std::ostream&
operator<< (std::ostream& os, const OutputNotification& on)
{
  os << (BaseNotification)on 
     << " sid = " << on.parent_session_id
     << " pack_size = " << on.pack_size;

  return os;
}



} // namespace dmcs

#endif // OUTPUT_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
