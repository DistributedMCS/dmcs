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

    /** 
     * Used to announce that context @a ctx_id has sent @a peq_cnt
     * partial equilibria.
     */
    struct JoinIn
    {
      std::size_t ctx_id;
      std::size_t peq_cnt;
    };


    /** 
     * Send @a m originating from @a from to @a to with priority @a prio. May block.
     * 
     * @param m a pointer to a MODEL
     * @param from sender
     * @param to receiver
     * @param prio priority
     */
    virtual void
    sendModel(MODEL* m, std::size_t from, std::size_t to, std::size_t prio) = 0;

    /** 
     * Send @a c originating from @a from to @a to with priority @a prio. May block.
     * 
     * @param c a pointer to a CONFLICT
     * @param from sender
     * @param to receiver
     * @param prio priority
     */
    virtual void
    sendConflict(CONFLICT* c, std::size_t from, std::size_t to, std::size_t prio) = 0;

    /** 
     * Send a JoinIn message from @a from to @a to with priority @a prio. Must not block.
     * 
     * @param k we have k partial equilibria ready to retrieve
     * @param from sender
     * @param to receiver
     * @param prio priority
     */
    virtual void
    sendJoinIn(std::size_t k, std::size_t from, std::size_t to, std::size_t prio) = 0;

    /** 
     * Receive a pointer to a MODEL from @a from. May block.
     * 
     * @param from sender
     * @param prio priority
     * 
     * @return the next pointer to the MODEL sent from @a from
     */
    virtual MODEL*
    recvModel(std::size_t from, std::size_t& prio) = 0;

    /** 
     * Receive a pointer to a CONFLICT from @a from. May block.
     * 
     * @param from sender
     * @param prio priority
     * 
     * @return the next pointer to the CONFLICT sent from @a from
     */
    virtual CONFLICT*
    recvConflict(std::size_t from, std::size_t& prio) = 0;

    /** 
     * Receive a JoinIn message from @a from to @a to with priority @a prio. May block.
     * 
     * @param from sender
     * @param prio priority
     *
     * @return the next JoinIn message
     */
    virtual struct JoinIn
    recvJoinIn(std::size_t from, std::size_t& prio) = 0;
    
  };

} // namespace dmcs


#endif // _MESSAGING_GATEWAY_H

// Local Variables:
// mode: C++
// End:

