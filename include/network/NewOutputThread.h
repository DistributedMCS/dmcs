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
 * @file   NewOutputThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu March  1 10:58:21 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_OUTPUT_THREAD_H
#define NEW_OUTPUT_THREAD_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "network/connection.hpp"
#include "network/NewConcurrentMessageDispatcher.h"
#include "network/NewOutputDispatcher.h"

namespace dmcs {

class NewOutputThread
{
public:
  NewOutputThread(std::size_t p,
		  std::size_t iid);

  ~NewOutputThread();

  void
  startup(connection_ptr c,
	  NewConcurrentMessageDispatcherPtr md);


  void
  init_mq(NewConcurrentMessageDispatcherPtr md,
	  NewOutputDispatcherPtr od);

  std::size_t
  getOutputOffset();


private:
  void
  write_result(connection_ptr c,
	       ReturnedBeliefStateListPtr output_list);

  void
  clean_up(ReturnedBeliefStateListPtr output_list);

private:
  bool initialized;
  std::size_t port;
  std::size_t invoker_id; // id of the context on which the connection through this output thread is established
  std::size_t offset; // offset of the corresponding concurrent message queue in md
};


typedef boost::shared_ptr<NewOutputThread> NewOutputThreadPtr;


struct NewOutputWrapper
{
  void
  operator()(NewOutputThreadPtr output_sender,
	     connection_ptr conn,
	     NewConcurrentMessageDispatcherPtr md)
  {
    output_sender->startup(conn, md);
  }
};

} // namespace dmcs

#endif // NEW_OUTPUT_THREAD_H

// Local Variables:
// mode: C++
// End:
