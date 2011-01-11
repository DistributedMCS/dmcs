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

#include "network/JoinThread.h"
#include "network/NeighborInputThread.h"
#include "network/OutputThread.h"
#include "network/RelSatSolverThread.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {

ThreadFactory::ThreadFactory(const ContextPtr& context_, const TheoryPtr& theory_,
			     const SignaturePtr& local_sig_,
			     const BeliefStatePtr& localV_,
			     std::size_t pack_size_,
			     boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : context(context_), theory(theory_), 
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
      NeighborInputThread nits(nb, ctx_id, pack_size, i, system_size, mg);
      
      boost::thread* nit = new boost::thread(nits);
      neighbor_input_threads->push_back(nit);
    }
}
  
boost::thread*
ThreadFactory::createJoinThread()
{
  const NeighborListPtr neighbors = context->getNeighbors();
  std::size_t no_nbs = neighbors->size();

  JoinThread dts(no_nbs, mg);
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

  RelSatSolverThread lts(relsatsolver);
  boost::thread* t = new boost::thread(lts);

  return t;
}


boost::thread*
ThreadFactory::createOutputThread(const connection_ptr& conn)
{
  OutputThread ots(conn, pack_size, mg);
  boost::thread* t = new boost::thread(ots);

  return t;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
