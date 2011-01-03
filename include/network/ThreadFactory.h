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

#include "network/Client.h"

#include "dmcs/StreamingCommandType.h"
#include "dmcs/StreamingDMCS.h"
#include "dmcs/StreamingForwardMessage.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

typedef std::vector<boost::thread*> ThreadVec;
typedef boost::shared_ptr<ThreadVec> ThreadVecPtr;

class NeighborInputThreadStarter
{
public:
  NeighborInputThreadStarter(const NeighborPtr& nb_, std::size_t ctx_id_);

  void
  operator()();

private:
  const NeighborPtr& nb;
  std::size_t ctx_id;
};



class DMCSThreadStarter
{
public:
  DMCSThreadStarter(StreamingDMCSPtr& sdmcs_);

  void 
  operator()();

private:
  StreamingDMCSPtr& sdmcs;
};



class LocalSolverThreadStarter
{
public:
  LocalSolverThreadStarter(StreamingDMCSPtr& sdmcs_);

  void
  operator()();

private:
  // may be we need less than sdmcs
  StreamingDMCSPtr& sdmcs;
};



class OutputThreadStarter
{
public:
  OutputThreadStarter();

  void
  operator()();
};



class ThreadFactory
{
public:
  ThreadFactory(StreamingDMCSPtr sdmcs_);

  void
  createNeighborInputThreads(ThreadVecPtr);

  boost::thread*
  createDMCSThread();

  boost::thread*
  createLocalSolveThread();

  boost::thread*
  createOutputThread();

private:
  StreamingDMCSPtr sdmcs;
};


} // namespace dmcs


#endif // THREAD_FACTORY_H

// Local Variables:
// mode: C++
// End:
