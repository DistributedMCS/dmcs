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
 * @file   ThreadFactory.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  3 12:39:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef THREAD_FACTORY_H
#define THREAD_FACTORY_H

#include "dmcs/Context.h"
#include "mcs/ProxySignatureByLocal.h"
#include "mcs/Theory.h"
#include "network/connection.hpp"
#include "solver/RelSatSolver.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

typedef std::vector<boost::thread*> ThreadVec;
typedef boost::shared_ptr<ThreadVec> ThreadVecPtr;

class NeighborInputThreadStarter
{
public:
  NeighborInputThreadStarter(const NeighborPtr& nb_, 
			     std::size_t ctx_id_, 
			     std::size_t pack_size_,
			     std::size_t system_size_,
			     boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_);

  void
  operator()();

private:
  const NeighborPtr nb;
  std::size_t ctx_id;
  std::size_t pack_size;
  std::size_t system_size;
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;
};



class JoinThreadStarter
{
public:
  JoinThreadStarter(boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_);

  void 
  operator()();

private:
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;
};



class LocalSolverThreadStarter
{
public:
  LocalSolverThreadStarter(const RelSatSolverPtr& relsatsolver_);

  void
  operator()();

private:
  RelSatSolverPtr relsatsolver;
};



class OutputThreadStarter
{
public:
  OutputThreadStarter(const connection_ptr& conn_,
		      std::size_t pack_size_,
		      boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_);

  void
  operator()();

  void
  collect_output(const boost::system::error_code& e);

  void
  write_result(const boost::system::error_code& e, BeliefStateVecPtr res);

private:
  const connection_ptr& conn;
  std::size_t pack_size;
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;  
};



class ThreadFactory
{
public:
  ThreadFactory(const ContextPtr& context_, const TheoryPtr& theory_,
		//		const ProxySignatureByLocalPtr& mixed_sig_,
		const SignaturePtr& local_sig_,
		const BeliefStatePtr& localV_,
		std::size_t pack_size_,
		boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_);

  void
  createNeighborInputThreads(ThreadVecPtr);

  boost::thread*
  createJoinThread();

  boost::thread*
  createLocalSolveThread();

  boost::thread*
  createOutputThread(const connection_ptr& conn_);

private:
  const ContextPtr               context;
  const TheoryPtr                theory;
  //  const ProxySignatureByLocalPtr mixed_sig;
  const SignaturePtr             local_sig;
  const BeliefStatePtr           localV;
  std::size_t                    pack_size;
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;
};


} // namespace dmcs


#endif // THREAD_FACTORY_H

// Local Variables:
// mode: C++
// End:
