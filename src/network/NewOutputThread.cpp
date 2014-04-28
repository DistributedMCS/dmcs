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
 * @file   NewOutputThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu March  1 11:20:01 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BackwardMessage.h"
#include "mcs/Logger.h"
#include "network/NewOutputThread.h"

namespace dmcs {

NewOutputThread::NewOutputThread(std::size_t p,
				 std::size_t iid)
  : initialized(false), 
    port(p), 
    invoker_id(iid)
{ }



NewOutputThread::~NewOutputThread()
{ }



void
NewOutputThread::init_mq(NewConcurrentMessageDispatcherPtr md,
			 NewOutputDispatcherPtr od)
{
  if (!initialized)
    {      
      offset = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::OUTPUT_MQ);
      DBGLOG(DBG, "NewOutputThread::init_mq(): For invoker = " << invoker_id << ", got offset = " << offset);
      initialized = true;
    }
  od->registerIdOffset(invoker_id, offset);
}



std::size_t
NewOutputThread::getOutputOffset()
{
  return offset;
}

void
NewOutputThread::clean_up(ReturnedBeliefStateListPtr output_list)
{
  for (ReturnedBeliefStateList::const_iterator it = output_list->begin();
       it != output_list->end(); ++it)
    {
      ReturnedBeliefState* rbs = *it;
      NewBeliefState* bs = rbs->belief_state;
      
      delete bs;
      bs = 0;
      
      delete rbs;
      rbs = 0;
    }
  output_list->clear();
}


void
NewOutputThread::startup(connection_ptr conn,
			 NewConcurrentMessageDispatcherPtr md)
{
  ReturnedBeliefStateListPtr output_list(new ReturnedBeliefStateList);

  int timeout = 0;
  while (1)
    {
      ReturnedBeliefState* res = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::OUTPUT_MQ, offset, timeout);

      if (res == NULL) 
	{
	  DBGLOG(DBG, "NewOutputThread::startup(): got NULL. BREAK NOW!");
	  clean_up(output_list);
	  break;
	}

      std::size_t iid = ctxid_from_qid(res->qid);
      assert (iid == invoker_id);

      output_list->push_back(res);
      
      if (res->belief_state == NULL) write_result(conn, output_list);
    }
}



void
NewOutputThread::write_result(connection_ptr conn,
			      ReturnedBeliefStateListPtr output_list)
{
  try 
    {
      const std::string header = HEADER_ANS;
      conn->write(header);

      output_list->sort(my_compare);
      remove_duplication(output_list);
      BackwardMessage bmess(output_list);
      
      DBGLOG(DBG, "NewOutThread::write_result(): to invoker: bmess = " << bmess);

      conn->write(bmess);

      clean_up(output_list);
    }
  catch (boost::system::error_code& e)
    {
      throw std::runtime_error(e.message());
    }
}

} // namespace dmcs


// Local Variables:
// mode: C++
// End:
