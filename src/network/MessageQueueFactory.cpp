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
 * @file   MessageQueueFactory.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Jan  4 11:25:59 2011
 * 
 * @brief  
 * 
 * 
 */


#include "network/MessageQueueFactory.h"
#include "network/MessageDispatcher.h"
#include "network/MessageQueue.h"

#include <sstream>

using namespace dmcs;

/// name of the conflict input MQ, announces conflicts from the parent
#define DMCS_IN_MQ           "dmcs-in-mq-"
/// name of the output MQ, announces partial equilibria
#define DMCS_OUT_MQ          "dmcs-out-mq-"
/// name of the conflict output MQ, announces new conflicts from the local solver
#define DMCS_CONFLICT_OUT_MQ "dmcs-conflict-out-mq-"

/// name of the join output MQ, announces joined belief states from the neighbors
#define DMCS_JOIN_OUT_MQ     "dmcs-join-out-mq-"
/// name of the join input MQ, announces that neighbor C_i sent k partial belief states
#define DMCS_JOIN_IN_MQ      "dmcs-join-in-mq-"

/// name of the partial equilibrium input MQ for neighbor C_i
#define DMCS_NEIGHBOR_IN_MQ  "dmcs-neighbor-in-mq-"
/// name of the conflict input MQ, announces a new conflict to a neighbor C_i
#define DMCS_CONFLICT_IN_MQ  "dmcs-conflict-in-mq-"




MessageQueueFactory::MessageQueueFactory()
{ }


MessageQueueFactory::~MessageQueueFactory()
{
  ///@todo TK: what to do here?
}

MQPtr
MessageQueueFactory::createMessageQueue(const char* name, std::size_t max_k, std::size_t max_size)
{
  MQPtr mq(new MQ(boost::interprocess::open_or_create, name, max_k, max_size));
  return mq;
}


boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >
MessageQueueFactory::createMessagingGateway(std::size_t uid, std::size_t no_nbs)
{
  ///@todo TK: all MQs need to have a unique name on the system, thus
  ///we need to add the process ID of the creating process to the name
  ///and announce this PID to child processes/threads. on dmcs
  ///shutdown, we need to cleanup using MQ::remove(). note: all MQs
  ///will be created under /tmp/boost_interprocess and stay alive
  ///after program execution.

  boost::shared_ptr<MessageDispatcher> md(new MessageDispatcher);
  MQPtr mq;
  std::ostringstream oss;

  ///@todo fixme, code duplication as well...
  std::size_t k = 5;

  // create message queue 0

  oss << DMCS_IN_MQ << uid;
  const std::string& name0 = oss.str();
  mq = createMessageQueue(name0.c_str(), k, sizeof(Conflict*));
  md->registerMQ(mq, name0);
  oss.str("");

  // create message queue 1
  
  oss << DMCS_OUT_MQ << uid;
  const std::string& name1 = oss.str();
  mq = createMessageQueue(name1.c_str(), k, sizeof(MessagingGateway<BeliefState,Conflict>::ModelConflict));
  md->registerMQ(mq, name1);
  oss.str("");

  // create message queue 2

  oss << DMCS_CONFLICT_OUT_MQ << uid;
  const std::string& name2 = oss.str();
  mq = createMessageQueue(name2.c_str(), k, sizeof(Conflict*));
  md->registerMQ(mq, name2);
  oss.str("");

  // create message queue 3

  oss << DMCS_JOIN_OUT_MQ << uid;
  const std::string& name3 = oss.str();
  mq = createMessageQueue(name3.c_str(), k, sizeof(BeliefState*));
  md->registerMQ(mq, name3);
  oss.str("");

  // create message queue 4

  oss << DMCS_JOIN_IN_MQ << uid;
  const std::string& name4 = oss.str();
  mq = createMessageQueue(name4.c_str(), k, sizeof(MessagingGateway<BeliefState,Conflict>::JoinIn));
  md->registerMQ(mq, name4);
  oss.str("");

  // create message queues 5 to 5+2*no_nbs

  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      oss << DMCS_NEIGHBOR_IN_MQ << i << '-' << uid;
      mq = createMessageQueue(oss.str().c_str(), k, sizeof(BeliefState*));
      md->registerMQ(mq, oss.str());
      oss.str("");

      oss << DMCS_CONFLICT_IN_MQ << i + no_nbs << '-' << uid;
      mq = createMessageQueue(oss.str().c_str(), k, sizeof(Conflict*));
      md->registerMQ(mq, oss.str());
      oss.str("");
    }

  return md;
}



boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >
MessageQueueFactory::createMessagingGateway(std::size_t uid)
{
  ///@todo TK: all MQs need to have a unique name on the system, thus
  ///we need to add the process ID of the creating process to the name
  ///and announce this PID to child processes/threads. on dmcs
  ///shutdown, we need to cleanup using MQ::remove(). note: all MQs
  ///will be created under /tmp/boost_interprocess and stay alive
  ///after program execution.

  boost::shared_ptr<MessageDispatcher> md(new MessageDispatcher);
  MQPtr mq;
  std::ostringstream oss;

  ///@todo fixme
  std::size_t k = 5;

  // create message queue 0

  oss << DMCS_IN_MQ << uid;
  const std::string& name0 = oss.str();
  mq = createMessageQueue(name0.c_str(), k, sizeof(Conflict*));
  md->registerMQ(mq, name0);
  oss.str("");

  // create message queue 1
  
  oss << DMCS_OUT_MQ << uid;
  const std::string& name1 = oss.str();
  mq = createMessageQueue(name1.c_str(), k, sizeof(MessagingGateway<BeliefState,Conflict>::ModelConflict));
  md->registerMQ(mq, name1);
  oss.str("");

  return md;
}





// Local Variables:
// mode: C++
// End:
