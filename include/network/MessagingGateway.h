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
  template <typename MODEL, typename DECISIONLEVEL, typename CONFLICT, typename MESSAGE>
  class MessagingGateway
  {
  public:

    ///@todo cleanup
#if 0
    /**
     * Used to announce that model @a m has a decisionlevel @a d.
     */
    struct ModelDecisionlevel
    {
      MODEL* m;
      DECISIONLEVEL* d;
    };


    /** 
     * Used to announce that model @a m has been computed
     * w.r.t. conflict @a c.
     */
    struct ModelConflict
    {
      MODEL* m;
      CONFLICT* c;
    };
#endif//0


    /** 
     * Used to announce that model @a m has been computed
     * w.r.t. session @a sid.
     */
    struct ModelSession
    {
      MODEL* m;
      std::size_t path; // the hash value of the path
      std::size_t sid;
    };

    /** 
     * Used to announce that context @a ctx_id has sent @a peq_cnt
     * partial equilibria for session @a sid.
     */
    struct JoinIn
    {
      std::size_t ctx_offset;
      std::size_t peq_cnt;
    };

    virtual bool
    sendIncomingMessage(MESSAGE* m, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0) = 0;

    /** 
     * Send @a m w.r.t. @a sid originating from @a from to @a to with priority @a prio. May block.
     * 
     * @param m a pointer to a MODEL
     * @param sid session id
     * @param from sender
     * @param to receiver
     * @param prio priority
     * @param msecs timed and try send with msecs > 0 and msecs < 0, resp.
     *
     * @return true if sending failed
     */
    virtual bool
    sendModel(MODEL* m, std::size_t path, std::size_t sid, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0) = 0;


#if 0
    /** 
     * Send model @a m w.r.t. conflict @a c originating from @a from to @a to with priority @a prio. May block.
     * 
     * @param m a pointer to a MODEL
     * @param from sender
     * @param to receiver
     * @param prio priority
     * @param msecs timed and try send with msecs > 0 and msecs < 0, resp.
     *
     * @return true if sending failed
     */
    virtual bool
    sendModelConflict(MODEL* m, CONFLICT* c, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0) = 0;


    /** 
     * Send model @a m w.r.t. decisionlevel @a d originating from @a from to @a to with priority @a prio. May block.
     * 
     * @param m a pointer to a MODEL
     * @param d a pointer to a DECISIONLEVEL
     * @param from sender
     * @param to receiver
     * @param prio priority
     * @param msecs timed and try send with msecs > 0 and msecs < 0, resp.
     *
     * @return true if sending failed
     */
    virtual bool
    sendModelDecisionlevel(MODEL* m, DECISIONLEVEL* d, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0) = 0;
#endif//0


    /** 
     * Send a JoinIn message from @a from to @a to with priority @a prio. Must not block.
     * 
     * @param k we have k partial equilibria ready to retrieve
     * @param sid session id
     * @param from sender
     * @param to receiver
     * @param prio priority
     * @param msecs timed and try send with msecs > 0 and msecs < 0, resp.
     *
     * @return true if sending failed
     */
    virtual bool
    sendJoinIn(std::size_t k, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0) = 0;

    virtual MESSAGE*
    recvIncomingMessage(std::size_t from, std::size_t& prio, int& msecs) = 0;

    /** 
     * Receive a ModelSession message from @a from. May block.
     * 
     * @param from sender
     * @param prio priority
     * @param msecs timed and try receive with msecs > 0 and msecs < 0, resp.
     * 
     * @return the next pointer to the MODEL sent from @a from
     */
    virtual struct ModelSession
    recvModel(std::size_t from, std::size_t& prio, int& msecs) = 0;


#if 0
    /** 
     * Receive a ModelConflict message from @a from with priority @a prio. May block.
     * 
     * @param from sender
     * @param prio priority
     * @param msecs timed and try receive with msecs > 0 and msecs < 0, resp.
     */
    virtual struct ModelConflict
    recvModelConflict(std::size_t from, std::size_t& prio, int& msecs) = 0;


    /** 
     * Receive a ModelDecisionlevel message from @a from with priority @a prio. May block.
     * 
     * @param from sender
     * @param prio priority
     * @param msecs timed and try receive with msecs > 0 and msecs < 0, resp.
     */
    virtual struct ModelDecisionlevel
    recvModelDecisionlevel(std::size_t from, std::size_t& prio, int& msecs) = 0;
#endif //0

    /** 
     * Receive a JoinIn message from @a from with priority @a prio. May block.
     * 
     * @param from sender
     * @param prio priority
     * @param msecs timed and try receive with msecs > 0 and msecs < 0, resp.
     *
     * @return the next JoinIn message
     */
    virtual struct JoinIn
    recvJoinIn(std::size_t from, std::size_t& prio, int& msecs) = 0;
    
  };

} // namespace dmcs


#endif // _MESSAGING_GATEWAY_H

// Local Variables:
// mode: C++
// End:

