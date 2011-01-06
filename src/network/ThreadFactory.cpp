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



DMCSThreadStarter::DMCSThreadStarter(StreamingDMCSPtr& sdmcs_)
  : sdmcs(sdmcs_)
{ }



void
DMCSThreadStarter::operator()()
{
#ifdef DEBUG
  std::cerr << "DMCSThreadStarter::operator()()" << std::endl;
#endif
  sdmcs->start_up();
}



LocalSolverThreadStarter::LocalSolverThreadStarter(const RelSatSolverPtr& relsatsolver_)
  : relsatsolver(relsatsolver_)
{ }



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



ThreadFactory::ThreadFactory(StreamingDMCSPtr& sdmcs_)
  : sdmcs(sdmcs_)
{ }


void
ThreadFactory::createNeighborInputThreads(ThreadVecPtr neighbor_input_threads)
{
  ContextPtr ctx = sdmcs->getContext();
  NeighborListPtr neighbors = ctx->getNeighbors();
  std::size_t ctx_id = ctx->getContextID();
  std::size_t system_size = ctx->getSystemSize();

  for (NeighborList::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
    {
      const NeighborPtr nb = *it;
      NeighborInputThreadStarter nits(nb, ctx_id, system_size);
      
      boost::thread* nit = new boost::thread(nits);
      neighbor_input_threads->push_back(nit);
    }
}
  
boost::thread*
ThreadFactory::createDMCSThread()
{
  DMCSThreadStarter dts(sdmcs);
  boost::thread* t = new boost::thread(dts);

  return t;
}
  
boost::thread*
ThreadFactory::createLocalSolveThread()
{
  ContextPtr   context        = sdmcs->getContext();
  TheoryPtr    theory         = sdmcs->getTheory();
  QueryPlanPtr query_plan     = sdmcs->getQueryPlan();
  SignatureVecPtr global_sigs = sdmcs->getGlobalSigs();
  SatSolverFactory ssf(context, theory, query_plan, global_sigs);

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
