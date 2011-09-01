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

#include "dmcs/Log.h"
#include "mcs/ResourceManager.h"

namespace dmcs {

ResourceManager::ResourceManager(std::size_t mr,
				 ThreadFactory* tf)
  : free_resource(0),
    max_resource(mr),
    thread_factory(tf),
    workers(new WorkerVec)
{ }



ConcurrentMessageQueue*
ResourceManager::createWorker(std::size_t path)
{
  WorkerPtr wk = thread_factory->createWorkerThreads(path, this, workers->size());
  
  workers->push_back(wk);
  wk->busy = true;
  
  DMCS_LOG_TRACE("Worker created, return request_mq = " << wk->request_mq);
  return wk->request_mq;
}



ConcurrentMessageQueue*
ResourceManager::requestWorker(std::size_t path, std::size_t k1, std::size_t k2)
{
  DMCS_LOG_TRACE("Got a request for worker");
  // make sure that only 1 requestWorer() is called at a time
  boost::mutex::scoped_lock lock(request_mtx);

  // if there is a worker that caches models from k1 to k2 then returns it
  for (WorkerVec::const_iterator it = workers->begin(); it != workers->end(); ++it)
    {
      WorkerPtr wk = *it;
      if ((!wk->busy) && (wk->k1 == k1) && (wk->k2 == k2))
	{
	  wk->busy = true;

	  {
	    boost::mutex::scoped_lock lock(mtx);
	    assert (free_resource > 0);
	    free_resource--;
	  }

	  DMCS_LOG_TRACE("Found a free worker. Return request_mq = " << wk->request_mq);
	  return wk->request_mq;
	}
    }

  if (workers->size() < max_resource)
    {
      DMCS_LOG_TRACE("Still have some resource left, going to create a new worker. path = " << path);
      return createWorker(path);
    }

  DMCS_LOG_TRACE("Wait until there is a free resource");
  {
    boost::mutex::scoped_lock lock(mtx);
    while (free_resource == 0)
      {
	cond.wait(lock);
      }
  }
  
  // find the free resource and return the corresponding ConcurrentMessageQueue
  WorkerVec::const_iterator it;
  for (it = workers->begin(); it != workers->end(); ++it)
    {
      if (!(*it)->busy)
	{
	  break;
	}
    }

  assert (it != workers->end());
  (*it)->busy = true;

  {
    boost::mutex::scoped_lock lock(mtx);
    assert (free_resource > 0);
    free_resource--;
  }

  DMCS_LOG_TRACE("Going to return request_mq = " << (*it)->request_mq);
  return (*it)->request_mq;
}



void
ResourceManager::updateStatus(std::size_t index, 
			      ConcurrentMessageQueue* request_mq,
			      bool bs, 
			      std::size_t k_one, 
			      std::size_t k_two)
{
  DMCS_LOG_TRACE("Going to update status for worker[" << index << "]. busy = " << bs);
  assert (workers->size() > index);
  WorkerVec::iterator it = workers->begin();
  std::advance(it, index);

  WorkerPtr wk = *it;

  // it's never the case that a worker's status is updated 
  // here and in requestWorker() simultaneously, hence we don't
  // need a mutex lock for this operation
  assert (request_mq == wk->request_mq);
  wk->busy = bs;
  wk->k1 = k_one;
  wk->k2 = k_two;

  if (bs == false)
    {
      boost::mutex::scoped_lock lock(mtx);
      free_resource++;
      cond.notify_one();
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
