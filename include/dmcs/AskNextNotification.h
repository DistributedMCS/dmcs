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
 * @file   AskNextNotification.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Mar  24 12:12:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ASK_NEXT_NOTIFICATION_H
#define ASK_NEXT_NOTIFICATION_H



#include "dmcs/BaseNotification.h"



namespace dmcs {



struct AskNextNotification : public BaseNotification
{
  AskNextNotification(BaseNotification::NotificationType t,
#ifdef DEBUG
		      History pa,
#else
		      std::size_t pa,
#endif
		      std::size_t sid,
		      std::size_t k_one,
		      std::size_t k_two)
    : BaseNotification(t, pa), 
      session_id(sid),
      k1(k_one),
      k2(k_two)
  { }
  
  std::size_t session_id;
  std::size_t k1;
  std::size_t k2;
};



inline std::ostream&
operator<< (std::ostream& os, const AskNextNotification& ann)
{
  os << (BaseNotification)ann;

  os << " session_id = " << ann.session_id 
     << " k1 = " << ann.k1
     << " k2 = " << ann.k2;

  return os;
}



} // namespace dmcs

#endif // ASK_NEXT_NOTIFICATION_H

// Local Variables:
// mode: C++
// End:
