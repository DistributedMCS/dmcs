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
 * @file   ThreadFactory.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  3 15:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/StreamingForwardMessage.h"
#include "dmcs/StreamingCommandType.h"
#include "network/Client.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {


NeighborInputThreadStarter::NeighborInputThreadStarter(const NeighborPtr& nb_, 
						       std::size_t ctx_id_,
						       std::size_t system_size_)
  : nb(nb_),
    ctx_id(ctx_id_),
    system_size(system_size_)
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
    StreamingForwardMessage neighbourMess(ctx_id, system_size);
    
    Client<StreamingCommandType> client(io_service, res_it, header, neighbourMess);
    
    io_service.run();
}



JoinThreadStarter::JoinThreadStarter()
{ }



void
JoinThreadStarter::operator()()
{
#ifdef DEBUG
  std::cerr << "JoinThreadStarter::operator()()" << std::endl;
#endif
}



LocalSolverThreadStarter::LocalSolverThreadStarter(const RelSatSolverPtr& relsatsolver_)
  : relsatsolver(relsatsolver_)
{
  std::cerr << "LocalSolverThreadStarter::ctor" << std::endl;
  relsatsolver->show();
}



void
LocalSolverThreadStarter::operator()()
{
  // (1) look into the message queue of the joined input, 
  // (2) pick the first BeliefState
  // (3) feed input into SATSolver
  // (4) solve and output to the output message queue

#ifdef DEBUG
  std::cerr << "LocalSolverThreadStarter::operator()()" << std::endl;
#endif

  // (3)
  relsatsolver->show();
  relsatsolver->solve();
}


// wait for MQGateway
OutputThreadStarter::OutputThreadStarter()
{ }



void
OutputThreadStarter::operator()()
{
#ifdef DEBUG
  std::cerr << "OutputThreadStarter::operator()()" << std::endl;
#endif
}



ThreadFactory::ThreadFactory(const ContextPtr& context_, const TheoryPtr& theory_,
			     const ProxySignatureByLocalPtr& mixed_sig_)
  : context(context_), theory(theory_), mixed_sig(mixed_sig_)
{ }


void
ThreadFactory::createNeighborInputThreads(ThreadVecPtr neighbor_input_threads)
{
  NeighborListPtr neighbors = context->getNeighbors();
  std::size_t ctx_id = context->getContextID();
  std::size_t system_size = context->getSystemSize();

  for (NeighborList::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
    {
      const NeighborPtr nb = *it;
      NeighborInputThreadStarter nits(nb, ctx_id, system_size);
      
      boost::thread* nit = new boost::thread(nits);
      neighbor_input_threads->push_back(nit);
    }
}
  
boost::thread*
ThreadFactory::createJoinThread()
{
  JoinThreadStarter dts;
  boost::thread* t = new boost::thread(dts);

  return t;
}
  
boost::thread*
ThreadFactory::createLocalSolveThread()
{
  std::size_t system_size = context->getSystemSize();
  SatSolverFactory ssf(theory, mixed_sig, system_size);

  RelSatSolverPtr relsatsolver = ssf.create<RelSatSolverPtr>();

  LocalSolverThreadStarter lts(relsatsolver);
  boost::thread* t = new boost::thread(lts);

  return t;
}


boost::thread*
ThreadFactory::createOutputThread()
{
  OutputThreadStarter ots;
  boost::thread* t = new boost::thread(ots);

  return t;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
