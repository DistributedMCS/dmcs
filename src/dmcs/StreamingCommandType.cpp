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
 * @file   StreamingCommandType.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  22 15:22:24 2010
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/StreamingCommandType.h"

namespace dmcs {

// Implemetation of methods that start threads
void
StreamingCommandType::createNeighborInputThreads(ThreadVecPtr neighbor_input_threads)
{
  ContextPtr ctx = sdmcs->getContext();
  NeighborListPtr neighbors = ctx->getNeighbors();
  std::size_t ctx_id = ctx->getContextID();

  for (NeighborList::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
    {
      const NeighborPtr nb = *it;
      NeighborInputThreadStarter nits(nb, ctx_id);
      
      boost::thread* nit = new boost::thread(nits);
      neighbor_input_threads->push_back(nit);
    }
}
  
void
StreamingCommandType::createDMCSThread(boost::thread*, const StreamingForwardMessage& mess)
{
}
  
void
StreamingCommandType::createLocalSolveThread(boost::thread*)
{
}

// Implementation for functors for threads
NeighborInputThreadStarter::NeighborInputThreadStarter(const NeighborPtr nb_, std::size_t ctx_id_)
  : nb(nb_),
    ctx_id(ctx_id_)
{ }


void
NeighborInputThreadStarter::operator()()
{
#if defined(DEBUG)
    std::cerr << "Send first request to neighbor " << nb->neighbor_id << "@" << nb->hostname << ":" << nb->port << std::endl;
#endif // DEBUG
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    
    
    boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
    
    boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint endpoint = *res_it;
    
    std::string header = HEADER_REQ_STM_DMCS;
    StreamingForwardMessage neighbourMess(ctx_id, true);
    
    Client<StreamingCommandType> client(io_service, res_it, header, neighbourMess);
    
    io_service.run();
}


DMCSThreadStarter::DMCSThreadStarter(StreamingDMCSPtr sdmcs_, const StreamingForwardMessage& mess_)
  : sdmcs(sdmcs_), mess(mess_)
{ }


void
DMCSThreadStarter::operator()()
{
  sdmcs->handleFirstRequest(mess);
}


LocalSolverThreadStarter::LocalSolverThreadStarter(StreamingDMCSPtr sdmcs_)
  : sdmcs(sdmcs_)
{ }


void
LocalSolverThreadStarter::operator()()
{
  // + look into the message queue of the joined input, 
  // + pick the first BeliefState
  // + feed input into SATSolver
  // + solve and output to the output message queue
}

} // namespace dmcs
