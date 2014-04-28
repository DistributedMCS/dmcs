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
 * @file   testMessageDispatcher.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Dec 29 17:25:23 2011
 * 
 * @brief  
 * 
 * 
 */


#include "network/NewConcurrentMessageDispatcher.h"
#include "mcs/ForwardMessage.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testMessageDispatcher"
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp> 
#include <boost/date_time.hpp>
#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testMessageDispatcher )
{
  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(5, 2));
  
  ForwardMessage* m1 = new ForwardMessage(1, 1, 10);
  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, m1);
  
  ForwardMessage* m2 = md->receive(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ);

  BOOST_CHECK_EQUAL(m1, m2);  
}

// Local Variables:
// mode: C++
// End:
