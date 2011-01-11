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
 * @file   OutputThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:30:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/StreamingForwardMessage.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "network/OutputThread.h"

namespace dmcs {

OutputThread::OutputThread(const connection_ptr& conn_,
					 std::size_t pack_size_,
					 boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : conn(conn_), pack_size(pack_size_), mg(mg_)
{
  std::cerr << "OutputThread::OutputThread(). pack_size = " << pack_size << std::endl; 
}



void
OutputThread::collect_output(const boost::system::error_code& e)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "OutputThread::collect_output(). pack_size = " << pack_size << std::endl;
#endif
      BeliefStateVecPtr res(new BeliefStateVec);
      
      // be careful with weird value of pack_size. Bug just disappreared
      for (std::size_t i = 0; i < pack_size; ++i)
	{
	  std::size_t prio = 0;
	  BeliefState* bs;
	  
	  std::cerr << "Read from MQ" << std::endl;
	  bs = mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
	  
	  if (bs == 0)
	    // either UNSAT of EOF
	    {
	      std::cerr << "OutputThread::collect_output(): NO MORE OUTPUT" << std::endl;
	      break;
	    }
	  
	  std::cerr << "got bs = " << *bs << std::endl;
	  res->push_back(bs);
	}
      
      std::string header;

      if (res->size() > 0)
	{
	  header = HEADER_ANS;
	  conn->async_write(header,
			    boost::bind(&OutputThread::write_result, this,
					boost::asio::placeholders::error, res)
			    );
	}
      else
	{
	  header = HEADER_EOF;
	  conn->async_write(header,
			    boost::bind(&OutputThread::collect_output, this,
					boost::asio::placeholders::error)
			    );
	}
    }
  else
    {
      // An error occurred.
#ifdef DEBUG
      std::cerr << "OutputThread::collect_output: " << e.message() << std::endl;
#endif
    }
}


void
OutputThread::write_result(const boost::system::error_code& e, BeliefStateVecPtr res)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "OutputThread::write_result()" << std::endl;
#endif

      StreamingBackwardMessage return_mess(res);
      
      conn->async_write(return_mess,
			boost::bind(&OutputThread::collect_output, this,
				    boost::asio::placeholders::error)
			);
    }
  else
    {
      // An error occurred.
#ifdef DEBUG
      std::cerr << "OutputThread::write_result: " << e.message() << std::endl;
#endif
    }
}



void
OutputThread::operator()()
{
#ifdef DEBUG
  std::cerr << "OutputThread::operator()()" << std::endl;
#endif

  collect_output(boost::system::error_code());
}




} // namespace dmcs

// Local Variables:
// mode: C++
// End:
