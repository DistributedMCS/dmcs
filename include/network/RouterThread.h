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
 * @file   RouterThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  14 19:17:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ROUTER_THREAD_H
#define ROUTER_THREAD_H

#include "dmcs/ConflictNotification.h"
#include "mcs/HashedBiMap.h"

#include <boost/thread.hpp>

namespace dmcs {

class RouterThread
{
public:
  RouterThread(ConflictNotificationFuturePtr& cnf_,
	       ConflictNotificationPromiseVecPtr& cnpv_,
	       HashedBiMapPtr& c2o_)
    : cnf(cnf_), cnpv(cnpv_), c2o(c2o_)
  { }

  void
  operator()()
  {
    while (1)
      {
	// wait for any conflict from the local solver
	cnf->wait();

	ConflictNotificationPtr cn = cnf->get();
	const std::size_t nid      = cn->val;
	
	DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Got a notification from local solver. nid = " << nid);

	// inform neighbor nid about the conflict
	const HashedBiMapByFirst& from_context  = boost::get<Tag::First>(*c2o);
	HashedBiMapByFirst::const_iterator pair = from_context.find(nid);
	const std::size_t noff                  = pair->second;
	ConflictNotificationPromisePtr cnp      = (*cnpv)[noff];

	cnp->set_value(cn);
      }
  }

private:
  ConflictNotificationFuturePtr     cnf;
  ConflictNotificationPromiseVecPtr cnpv;
  HashedBiMapPtr              c2o;
};

typedef boost::shared_ptr<RouterThread> RouterThreadPtr;

} // namespace dmcs


#endif // ROUTER_THREAD_H

// Local Variables:
// mode: C++
// End:
