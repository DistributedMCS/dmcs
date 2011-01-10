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
#include "dmcs/StreamingBackwardMessage.h"
#include "dmcs/StreamingCommandType.h"
#include "network/AsynClient.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {


NeighborInputThreadStarter::NeighborInputThreadStarter(const NeighborPtr& nb_, 
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
NeighborInputThreadStarter::operator()()
{
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *res_it;

  const std::string header = HEADER_REQ_STM_DMCS;
  
  client = new AsynClient<StreamingForwardMessage, StreamingBackwardMessage>(io_service, res_it, header, mg, nb, ctx_id, index, pack_size);

  io_service.run();
}



JoinThreadStarter::JoinThreadStarter(boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : mg(mg_)
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
}



void
LocalSolverThreadStarter::operator()()
{
#ifdef DEBUG
  std::cerr << "LocalSolverThreadStarter::operator()()" << std::endl;
#endif

  relsatsolver->solve();
}


OutputThreadStarter::OutputThreadStarter(const connection_ptr& conn_,
					 std::size_t pack_size_,
					 boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : conn(conn_), pack_size(pack_size_), mg(mg_)
{
  std::cerr << "OutputThreadStarter::OutputThreadStarter(). pack_size = " << pack_size << std::endl; 
}



///@todo: remove code duplication
void
OutputThreadStarter::collect_output(const boost::system::error_code& e)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "OutputThreadStarter::collect_output(). pack_size = " << pack_size << std::endl;
#endif
      BeliefStateVecPtr res(new BeliefStateVec);
      
      // be careful with weird value of pack_size. Bug just disappreared
      for (std::size_t i = 0; i < pack_size; ++i)
	{
	  std::size_t prio = 0;
	  BeliefState* bs;
	  
	  std::cerr << "Read from MQ" << std::endl;
	  bs = mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
	  
	  if (bs == 0)
	    // either UNSAT of EOF
	    {
	      std::cerr << "OutputThreadStarter::collect_output(): NO MORE OUTPUT" << std::endl;
	      break;
	    }
	  
	  std::cerr << "got bs = " << *bs << std::endl;
	  res->push_back(bs);
	}
      
      std::string header;

      if (res->size() > 0)
	{
	  header = HEADER_ANS;
	  conn->async_write(header,
			    boost::bind(&OutputThreadStarter::write_result, this,
					boost::asio::placeholders::error, res)
			    );
	}
      else
	{
	  header = HEADER_EOF;
	  conn->async_write(header,
			    boost::bind(&OutputThreadStarter::collect_output, this,
					boost::asio::placeholders::error)
			    );
	}
    }
  else
    {
      // An error occurred.
#ifdef DEBUG
      std::cerr << "OutputThreadStarter::collect_output: " << e.message() << std::endl;
#endif
    }
}


void
OutputThreadStarter::write_result(const boost::system::error_code& e, BeliefStateVecPtr res)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "OutputThreadStarter::write_result()" << std::endl;
#endif

      StreamingBackwardMessage return_mess(res);
      
      conn->async_write(return_mess,
			boost::bind(&OutputThreadStarter::collect_output, this,
				    boost::asio::placeholders::error)
			);
    }
  else
    {
      // An error occurred.
#ifdef DEBUG
      std::cerr << "OutputThreadStarter::write_result: " << e.message() << std::endl;
#endif
    }
}



void
OutputThreadStarter::operator()()
{
#ifdef DEBUG
  std::cerr << "OutputThreadStarter::operator()()" << std::endl;
#endif

  collect_output(boost::system::error_code());
}



ThreadFactory::ThreadFactory(const ContextPtr& context_, const TheoryPtr& theory_,
			     //   const ProxySignatureByLocalPtr& mixed_sig_,
			     const SignaturePtr& local_sig_,
			     const BeliefStatePtr& localV_,
			     std::size_t pack_size_,
			     boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : context(context_), theory(theory_), 
    //mixed_sig(mixed_sig_), 
    local_sig(local_sig_),
    localV(localV_),
    pack_size(pack_size_),
    mg(mg_)
{ }


void
ThreadFactory::createNeighborInputThreads(ThreadVecPtr neighbor_input_threads)
{
  NeighborListPtr neighbors = context->getNeighbors();
  std::size_t ctx_id = context->getContextID();
  std::size_t system_size = context->getSystemSize();

  std::size_t i = 0;
  for (NeighborList::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it, ++i)
    {
      const NeighborPtr nb = *it;
      NeighborInputThreadStarter nits(nb, ctx_id, pack_size, i, system_size, mg);
      
      boost::thread* nit = new boost::thread(nits);
      neighbor_input_threads->push_back(nit);
    }
}
  
boost::thread*
ThreadFactory::createJoinThread()
{
  JoinThreadStarter dts(mg);
  boost::thread* t = new boost::thread(dts);

  return t;
}
  
boost::thread*
ThreadFactory::createLocalSolveThread()
{
  std::size_t my_id = context->getContextID();
  std::size_t system_size = context->getSystemSize();
  SatSolverFactory ssf(my_id, theory, local_sig, localV, system_size, mg);

  RelSatSolverPtr relsatsolver = ssf.create<RelSatSolverPtr>();

  LocalSolverThreadStarter lts(relsatsolver);
  boost::thread* t = new boost::thread(lts);

  return t;
}


boost::thread*
ThreadFactory::createOutputThread(const connection_ptr& conn)
{
  OutputThreadStarter ots(conn, pack_size, mg);
  boost::thread* t = new boost::thread(ots);

  return t;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
