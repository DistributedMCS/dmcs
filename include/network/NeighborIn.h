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
 * @file   NeighborIn.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  13 22:26:21 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef NEIGHBOR_IN_H
#define NEIGHBOR_IN_H

#include "dmcs/Log.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "mcs/BeliefState.h"
#include "network/BaseStreamer.h"

namespace dmcs {

/**
 * @brief
 */
class NeighborIn : BaseStreamer
{
public:
  NeighborIn(connection_ptr& conn_, 
	     const MessagingGatewayBCPtr& mg_, 
	     std::size_t noff_)
    : BaseStreamer(conn_),
      mg(mg_), noff(noff_),
      first_round(true)
  {
    stream(boost::system::error_code());
  }

  void 
  stream(const boost::system::error_code& e) 
  {
	DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "noff = " << noff);
    if (!e)
      {
	// write to NEIGHBOR_IN_MQ, only from the second round
	if (first_round)
	  {
	    first_round = false;
	  }
	else
	  {
	    // extract a bunch of models from the message and then put each into NEIGHBOR_MQ
	    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Write to MQs");
	    const BeliefStateVecPtr bsv = mess.getBeliefStates();
	    const std::size_t offset    = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + noff;

	    for (BeliefStateVec::const_iterator it = bsv->begin(); it != bsv->end(); ++it)
	      {
		BeliefState* bs = *it;
		mg->sendModel(bs, noff, offset, 0);
	      }
	    // notify the joiner by putting a JoinMess into JoinMessageQueue
	    mg->sendJoinIn(bsv->size(), noff, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
	  }
	
	// read from network
	DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Reading from network");
	conn->async_read(mess, boost::bind(&NeighborIn::stream, this,  
					   boost::asio::placeholders::error));
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }

private:
  bool                     first_round;
  std::size_t              noff;        // offset of the neighbor streamer in the vector of MQs
  MessagingGatewayBCPtr    mg;
  StreamingBackwardMessage mess;
};

typedef boost::shared_ptr<NeighborIn> NeighborInPtr;

} // namespace dmcs

#endif // NEIGHBOR_IN_H

// Local Variables:
// mode: C++
// End:
