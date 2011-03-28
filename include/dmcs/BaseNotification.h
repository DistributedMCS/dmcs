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
 * @file   BaseNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Mar  21 8:58:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_NOTIFICATION_H
#define BASE_NOTIFICATION_H

#include "dmcs/BaseTypes.h"

namespace dmcs {

struct BaseNotification
{
  enum NotificationType
    {
      REQUEST = 0,
      NEXT,
      SHUTDOWN,
      SHUTUP
    };

  BaseNotification(NotificationType t, 
		   std::size_t pa)
    : type(t), path(pa)
  {
    assert(t == REQUEST || t == SHUTDOWN || t == NEXT); 
  }
  
  NotificationType type;
  std::size_t path;
};


inline std::ostream&
operator<< (std::ostream& os, const BaseNotification& bn)
{
  switch (bn.type)
    {
    case BaseNotification::REQUEST:
      {
	os << "REQUEST";
	break;
      }
    case BaseNotification::NEXT:
      {
	os << "NEXT";
	break;
      }

    case BaseNotification::SHUTDOWN:
      {
	os << "SHUTDOWN";
	break;
      }
    }

  os << " {" << bn.path << "}"; 

  return os;
}

} // namespace dmcs

#endif // BASE_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
