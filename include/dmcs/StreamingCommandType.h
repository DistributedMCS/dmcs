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
 * @file   StreamingCommandType.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  20  9:39:01 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_COMMAND_TYPE
#define STREAMING_COMMAND_TYPE

#include "CommandType.h"
#include "StreamingForwardMessage.h"
#include "StreamingDMCS.h"
//#include "network/Thread.h"
#include "network/Client.h"

#include <boost/thread.hpp>

namespace dmcs {

class SendingRequest;


class StreamingCommandType : public CommandType<StreamingForwardMessage, bool>
{
public:
  typedef StreamingForwardMessage input_type;
  typedef StreamingDMCS::dmcs_value_type value_type;

  StreamingCommandType(StreamingDMCSPtr sdmcs_)
    : sdmcs(sdmcs_)
  { }

  // return true/false indicating whether the execution was successful
  // while sending local models to message queue.
  bool
  execute(const StreamingForwardMessage& mess)
  {
    return sdmcs->handleFirstRequest(mess);
  }

  bool
  continues(StreamingForwardMessage& /* mess */)
  {
    return false;
  }

  void
  createNeighborInputThreads(ThreadVecPtr);

  void
  createDMCSThread(boost::thread*, const StreamingForwardMessage& mess);

  void
  createLocalSolveThread(boost::thread*);

private:
  StreamingDMCSPtr sdmcs;
};


typedef boost::shared_ptr<StreamingCommandType> StreamingCommandTypePtr;


// Functors for threads
class NeighborInputThreadStarter
{
public:
  NeighborInputThreadStarter(const NeighborPtr nb_, std::size_t ctx_id_);

  void
  operator()();

private:
  const NeighborPtr& nb;
  std::size_t ctx_id;
};



class DMCSThreadStarter
{
public:
  DMCSThreadStarter(StreamingDMCSPtr sdmcs_, const StreamingForwardMessage& mess_);

  void 
  operator()();

private:
  StreamingDMCSPtr sdmcs;
  const StreamingForwardMessage& mess;
};



class LocalSolverThreadStarter
{
public:
  LocalSolverThreadStarter(StreamingDMCSPtr sdmcs_);

  void
  operator()();

private:
  // may be we need less than sdmcs
  StreamingDMCSPtr sdmcs;
};

} // namespace dmcs

#endif // STREAMING_COMMAND_TYPE

// Local Variables:
// mode: C++
// End:
