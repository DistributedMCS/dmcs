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

#include "network/ConflictsAnalysisThread.h"
#include "network/JoinThread.h"
#include "network/NeighborThread.h"
#include "network/OutputThread.h"
#include "network/RelSatSolverThread.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {

ThreadFactory::ThreadFactory(const ContextPtr& context_, 
			     const TheoryPtr& theory_,
			     const SignaturePtr& local_sig_,
			     const BeliefStatePtr& localV_,
			     std::size_t pack_size_,
			     MessagingGatewayBCPtr& mg_)
  : context(context_), theory(theory_), 
    local_sig(local_sig_), localV(localV_),
    pack_size(pack_size_), mg(mg_),
    c2o(new HashedBiMap)
{
  // fill up the map: ctx_id <--> offset
  const NeighborListPtr& nbs = context->getNeighbors();
  std::size_t off = 0;
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++off)
    {
      std::size_t nid = (*it)->neighbor_id;
      c2o->insert(Int2Int(nid, off));
    }
}



void
ThreadFactory::createNeighborInputThreads(ThreadVecPtr neighbor_input_threads)
{
  NeighborListPtr& nbs          = context->getNeighbors();
  const std::size_t ctx_id      = context->getContextID();
  const std::size_t system_size = context->getSystemSize();

  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      const NeighborPtr nb = *it;
      NeighborThread nt(nb, c2o, ctx_id, pack_size, system_size, mg);
      
      boost::thread* nit = new boost::thread(nt);
      neighbor_input_threads->push_back(nit);
    }
}



boost::thread*
ThreadFactory::createJoinThread()
{
  const NeighborListPtr& nbs = context->getNeighbors();
  const std::size_t no_nbs   = nbs->size();

  JoinThread jt(no_nbs, c2o, mg, bnf);
  boost::thread* t = new boost::thread(jt);

  return t;
}



boost::thread*
ThreadFactory::createLocalSolveThread()
{
  const std::size_t my_id       = context->getContextID();
  const std::size_t system_size = context->getSystemSize();

  SatSolverFactory ssf(my_id, theory, local_sig, localV, system_size, mg);

  RelSatSolverPtr relsatsolver = ssf.create<RelSatSolverPtr>();

  RelSatSolverThread rsst(relsatsolver);
  boost::thread* t = new boost::thread(rsst);

  return t;
}



boost::thread*
ThreadFactory::createRouterThread()
{
  RouterThread rt();
  boost::thread* t = new boost::thread(rt);

  return t;
}


  /*
boost::thread*
ThreadFactory::createOutputThread(const connection_ptr& conn)
{
  OutputThread ots(conn, pack_size, mg, onf);
  boost::thread* t = new boost::thread(ots);

  return t;
  }*/

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
