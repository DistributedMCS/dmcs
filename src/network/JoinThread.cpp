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
 * @date   Tue Jan  11 9:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "network/JoinThread.h"

namespace dmcs {


JoinThread::JoinThread(std::size_t no_nbs_,
		       boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : no_nbs(no_nbs_),
    partial_eqs(new BeliefStatePackage(no_nbs_)),
    mg(mg_)
{ }



void
JoinThread::operator()()
{
#ifdef DEBUG
  std::cerr << "JoinThread::operator()()" << std::endl;
#endif
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
