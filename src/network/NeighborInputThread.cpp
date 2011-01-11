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
 * @file   NeighborInputThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 8:43:20 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/StreamingForwardMessage.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "dmcs/StreamingCommandType.h"
#include "network/AsynClient.h"
#include "network/NeighborInputThread.h"

namespace dmcs {


NeighborInputThread::NeighborInputThread(const NeighborPtr& nb_, 
					 std::size_t ctx_id_,
					 std::size_t pack_size_,
					 std::size_t index_,
					 std::size_t system_size_,
					 boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : nb(nb_),
    ctx_id(ctx_id_),
    pack_size(pack_size_),
    index(index_),
    system_size(system_size_),
    mg(mg_)
{

}


void
NeighborInputThread::operator()()
{
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *res_it;

  const std::string header = HEADER_REQ_STM_DMCS;
  
  client = new AsynClient<StreamingForwardMessage, StreamingBackwardMessage>(io_service, res_it, header, mg, 
									     nb, ctx_id, index, pack_size);

  io_service.run();
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
