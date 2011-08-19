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
 * @file   ResourceManager.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Aug  18 16:26:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "mcs/ResourceManager.h"

namespace dmcs {

ResourceManager::ResourceManager(std::size_t mr,
				 ThreadFactory* tf)
  : max_resource(mr),
    thread_factory(tf),
    workers(new WorkerVec)
{ }



ConcurrentMessageQueue*
ResourceManager::createWorker(std::size_t path)
{
  WorkerPtr wk = thread_factory->createWorkerThreads(path);
  wk->busy = true;
  
  workers->push_back(wk);

  return wk->request_mq;
}



ConcurrentMessageQueue*
ResourceManager::requestWorker(std::size_t path)
{
  boost::mutex::scoped_lock lock(mtx);
}



void
ResourceManager::updateStatus(std::size_t index, 
			      ConcurrentMessageQueue* request_mq,
			      bool bs, 
			      std::size_t k_one, 
			      std::size_t k_two)
{
  assert (workers->size() > index);
  WorkerVec::iterator it = workers->begin();
  std::advance(it, index);

  WorkerPtr wk = *it;

  assert (request_mq == wk->request_mq);
  wk->busy = bs;
  wk->k1 = k_one;
  wk->k2 = k_two;
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
