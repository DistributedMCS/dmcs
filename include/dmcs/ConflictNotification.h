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
 * @file   ConflictNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  14 18:42:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef CONFLICT_NOTIFICATION_H
#define CONFLICT_NOTIFICATION_H

#include "dmcs/BaseNotification.h"
#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

namespace dmcs {



struct ConflictNotification : public BaseNotification
{
  enum SenderIdentification
    {
      FROM_DMCS = 0,
      FROM_JOINER,
      FROM_ROUTER
    };

  ConflictNotification(BaseNotification::NotificationType t,
		       PathList pa,
		       std::size_t sid,
		       std::size_t ps,
		       SenderIdentification s)
    : BaseNotification(t, pa), 
      session_id(sid),
      pack_size(ps),
      sender(s)
  { }
  
  std::size_t session_id;
  std::size_t pack_size;
  SenderIdentification sender; // identifier of the thread that sent this 
};



inline std::ostream&
operator<< (std::ostream& os, const ConflictNotification& cn)
{
  os << (BaseNotification)cn;

  os << " session_id = " << cn.session_id 
     << " pack_size = " << cn.pack_size 
     << " sender = ";

  switch (cn.sender)
    {
    case ConflictNotification::FROM_DMCS:
      {
	os << "DMCS";
	break;
      }
    case ConflictNotification::FROM_JOINER:
      {
	os << "JOINER";
	break;
      }
    case ConflictNotification::FROM_ROUTER:
      {
	os << "ROUTER";
	break;
      }
    }

  return os;
}

} // namespace dmcs

#endif // CONFLICT_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
