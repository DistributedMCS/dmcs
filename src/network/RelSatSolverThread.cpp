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
 * @file   RelSatSolverThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  3 15:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/Log.h"
#include "network/RelSatSolverThread.h"

namespace dmcs {

RelSatSolverThread::RelSatSolverThread(const RelSatSolverPtr& rss)
  : relsatsolver(rss)
{ 
  DMCS_LOG_TRACE("Destructor.");
}

void
RelSatSolverThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  while (1)
    {
      try 
	{
	  relsatsolver->solve();
	}
      catch(const boost::thread_interrupted& ex)
	{
	  DMCS_LOG_TRACE("Got interrupted, will now restart.");
	  
	  // reset sat solver
	  
	}
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
