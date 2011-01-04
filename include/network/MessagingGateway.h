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
 * @file   MessagingGateway.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Dec 21 06:04:11 2010
 * 
 * @brief  Encapsulates the messaging system of DMCS.
 * 
 * 
 */



#ifndef _MESSAGING_GATEWAY_H
#define _MESSAGING_GATEWAY_H

#include <cstddef>

namespace dmcs {

  /** 
   * @brief Base class for sending and receiving models and conflicts
   * within a dmcsd.
   */
  template <typename MODEL, typename CONFLICT>
  class MessagingGateway
  {
  public:

    virtual void
    sendModel(MODEL*, std::size_t from, std::size_t to, std::size_t prio) = 0;

    virtual void
    sendConflict(CONFLICT*, std::size_t from, std::size_t to, std::size_t prio) = 0;

    virtual MODEL*
    recvModel(std::size_t from, std::size_t& prio) = 0;

    virtual CONFLICT*
    recvConflict(std::size_t from, std::size_t& prio) = 0;
    
  };

} // namespace dmcs


#endif // _MESSAGING_GATEWAY_H

// Local Variables:
// mode: C++
// End:

