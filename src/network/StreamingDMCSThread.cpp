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
 * @file   StreamingDMCSThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  12 16:48:59 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/Log.h"
#include "network/StreamingDMCSThread.h"

namespace dmcs {

  StreamingDMCSThread::StreamingDMCSThread(std::size_t p)
    : port(p)
  { }


StreamingDMCSThread::~StreamingDMCSThread()
{
  DMCS_LOG_TRACE(port << ": Gone with the wind.");
}


void
StreamingDMCSThread::operator()(StreamingCommandType* scmt, ConcurrentMessageQueue* nfh)
{
  //  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  scmt->execute(nfh);
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
