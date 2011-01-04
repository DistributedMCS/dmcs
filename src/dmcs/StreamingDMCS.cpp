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
 * @file   StreamingDMCS.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  20 10:11:24 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "mcs/BeliefState.h"
#include "dmcs/BeliefCombination.h"
#include "dmcs/CommandType.h"
#include "dmcs/Cache.h"
#include "loopformula/DimacsVisitor.h"

#include "dmcs/StreamingDMCS.h"
#include "dmcs/StreamingCommandType.h"


#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include "network/Client.h"
#include "network/MessageQueue.h"

#include <vector>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace dmcs {


StreamingDMCS::StreamingDMCS(const ContextPtr& c, const TheoryPtr& t, 
			     const SignatureVecPtr& s, 
			     const QueryPlanPtr& query_plan_,
			     std::size_t buf_count_)
  : BaseDMCS(c, t, s),
    query_plan(query_plan_),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats)),
    system_size(c->getSystemSize()),
    my_id(c->getContextID()),
    buf_count(buf_count_)
{ }



StreamingDMCS::~StreamingDMCS()
{ }


ContextPtr
StreamingDMCS::getContext()
{
  return ctx;
}


// Transfer local theory to internal clause representation of relsat
void
StreamingDMCS::init_SATSolver()
{
  for (Theory::const_iterator it = theory->begin(); it != theory->end(); ++it)
    {
      const ClausePtr& cl = *it;
      for (Clause::const_iterator jt = cl->begin(); jt != cl->end(); ++jt)
	{
	}
    }
}


SignaturePtr 
StreamingDMCS::createGuessingSignature(const BeliefStatePtr& V, const SignaturePtr& my_sig)
{
  SignaturePtr guessing_sig(new Signature);

  // local id in guessing_sig will start from my signature's size + 1
  std::size_t guessing_sig_local_id = my_sig->size() + 1;

  const SignatureBySym& my_sig_sym = boost::get<Tag::Sym>(*my_sig);

  const NeighborListPtr& neighbors = ctx->getNeighbors();

  for (NeighborList::const_iterator n_it = neighbors->begin(); n_it != neighbors->end(); ++n_it)
    {
      NeighborPtr nb = *n_it;
      const std::size_t neighbor_id = nb->neighbor_id;
      const BeliefSet neighbor_V = (*V)[neighbor_id - 1];
      const Signature& neighbor_sig = *((*global_sigs)[neighbor_id - 1]);

#ifdef DEBUG
      std::cerr << "Interface variable of neighbor[" << nb->neighbor_id <<"]: " << neighbor_V << std::endl;
#endif

      guessing_sig_local_id = updateGuessingSignature(guessing_sig,
						      my_sig_sym,
						      neighbor_sig,
						      neighbor_V,
						      guessing_sig_local_id);
    }
      
#ifdef DEBUG
    std::cerr << "Guessing signature: " << *guessing_sig << std::endl;
#endif

    return guessing_sig;
}



// initialize message queues 
void
StreamingDMCS::init_mqs()
{
  std::stringstream str_my_id;
  str_my_id << my_id;

  std::stringstream str_neighbor_id;
  std::string mq_label;

  // message queues for holding answers from neighbors
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);

#if 0
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      NeighborPtr nb = *it;

      str_neighbor_id.str();
      str_neighbor_id << nb->neighbor_id;

      mq_label = DMCS_IN_MQ "-" + str_my_id.str() + "-" + str_neighbor_id.str();
      
      MQ mq_in(boost::interprocess::create_only, 
	       mq_label.c_str(), buf_count, buf_count * DMCS_MQ_MSG_SIZE);
    }

  // message queue for joining input from neighbors
  mq_label = DMCS_JOIN_IN_MQ "-" + str_my_id.str();
  MQ mq_join(boost::interprocess::create_only,
	     mq_label.c_str(), buf_count, buf_count * DMCS_JOIN_IN_MSG_SIZE);
	     
  // message queue for holding local answers from SAT solver
  mq_label = DMCS_OUT_MQ "-" + str_my_id.str();
  MQ mq_out(boost::interprocess::create_only,
	    mq_label.c_str(), buf_count, buf_count * DMCS_MQ_MSG_SIZE);
#endif // 0
}



// remove all message queues
void
StreamingDMCS::remove_mqs()
{
  std::stringstream str_my_id;
  str_my_id << my_id;

  std::stringstream str_neighbor_id;
  std::string mq_label;

  // message queues for holding answers from neighbors
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);

#if 0
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      NeighborPtr nb = *it;

      str_neighbor_id.str();
      str_neighbor_id << nb->neighbor_id;

      mq_label = DMCS_IN_MQ "-" + str_my_id.str() + "-" + str_neighbor_id.str();
      
      MQ::remove(mq_label.c_str());
    }

  // message queue for joining input from neighbors
  mq_label = DMCS_JOIN_IN_MQ "-" + str_my_id.str();
  MQ::remove(mq_label.c_str());
	     
  // message queue for holding local answers from SAT solver
  mq_label = DMCS_OUT_MQ "-" + str_my_id.str();
  MQ::remove(mq_label.c_str());
#endif //0
}



bool
StreamingDMCS::handleFirstRequest(const StreamingForwardMessage& mess)
{
  const std::size_t c = mess.getInvoker();

#if defined(DEBUG)
  std::cerr << "context " << c << " is calling context " << my_id << std::endl;
#endif // DEBUG

  const NeighborListPtr& nb = query_plan->getNeighbors(my_id);
  
  if (nb->size() == 0) // this is a leaf context
    {
      // local compute without input
    }
  else // this is an intermediate context
    {
      // send request to neighbors
      //sendFirstRequest(nb);
    }
}




// create multiple threads to send requests to neighbors in
// parallel. Each thread then is responsible for receiving the
// BeliefState(s) from the respective neighbor.
/*void
StreamingDMCS::sendFirstRequest(const NeighborListPtr& nb)
{
  ThreadVec tv;

  for (NeighborList::const_iterator it = nb->begin(); it != nb->end(); ++it)
    {
      boost::asio::io_service io_service;
      boost::asio::ip::tcp::resolver resolver(io_service);

      const NeighborPtr nb = *it;

      SendSingleFirstRequest s(nb, my_id);

      boost::thread* sendingThread = new boost::thread(s);
      tv.push_back(sendingThread);
    }

#ifdef DEBUG
  std::cerr << "sendFirstRequest threads started. Now waiting..." << std::endl;
#endif

  for (ThreadVec::const_iterator it = tv.begin(); it != tv.end(); ++it)
    {
      (*it)->join();
    }

  // Now we have input from all neighbors, we can start the joining
  // business

  // Read off pointers to BeliefStates from message queues. 
  }*/



void
StreamingDMCS::localCompute(BeliefState* input, BeliefState* conflict)
{
  // remove input clauses from previous localCompute

  // add input and conflict (in the same shape of BeliefState, but have different meanings)
  // input: facts 

  // conflict: clause
}

} // namespace dmcs


// Local Variables:
// mode: C++
// End:
