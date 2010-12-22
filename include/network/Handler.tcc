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
 * @file   Handler.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Sep  29 07:23:24 2010
 * 
 * @brief  
 * 
 * 
 */


//#include "Handler.h"
#include "dmcs/StreamingDMCS.h"

namespace dmcs {

template<typename CmdType>
//Handler<CmdType>::Handler(const CmdTypePtr cmd_, connection_ptr conn_)
Handler<CmdType>::Handler(CmdTypePtr cmd, connection_ptr conn_)
//: cmd(cmd_), conn(conn_)
  : conn(conn_),
    neighbor_input_threads(new ThreadVec)
{
  std::cerr << "Handler::Handler, going to read message" << std::endl;
  
  SessionMsgPtr sesh(new SessionMsg(conn));

  // read and process this message
  conn->async_read(sesh->mess,
		   boost::bind(&Handler<CmdType>::do_local_job, this,
			       boost::asio::placeholders::error, sesh, cmd)
		   );
}



template<typename CmdType>
void
Handler<CmdType>::do_local_job(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {
#if defined(DEBUG)
      std::cerr << "Handler::do_local_job " << std::endl
		<< "Message = " << sesh->mess << std::endl;
#endif //DEBUG
      // create the message queue for holding results from local
      // solving. Then spawn another thread which keeps reading the
      // queue and returns each single computed BeliefState to the
      // invoker through the network. Since the receiver also gets the
      // results into a message queue, it will automatically block us
      // here when its queue is full.


      // initialize threads for: (i) receiving answers from
      // neighbor(s), (ii) joining partial equilibria from
      // neighbor(s), and (iii) SAT solving
      cmd->createNeighborInputThreads(neighbor_input_threads);
      cmd->createDMCSThread(dmcs_thread, sesh->mess);
      cmd->createLocalSolveThread(sat_thread);

	  
      // asynchronously look at the output message queue, pick up each
      // new coming BeliefState and send back to the invoker. Using
      // boost::asio, we need to loop between functions using
      // boost::bind(), and of course outside of this do_local_job()
      send_header_result(e, sesh, cmd);
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "Handler::do_local_job: " << e.message() << std::endl;
#endif
    }
}



/// MD: watch out! We might lose cmd if we don't pass it as an argument. This happened before.
template<typename CmdType>
void
Handler<CmdType>::send_header_result(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "Handler::send_header_result" << std::endl;
#endif

      std::string header = HEADER_ANS;

      sesh->conn->async_write(header,
			      boost::bind(&Handler<CmdType>::send_result, this,
					  boost::asio::placeholders::error, sesh, cmd));
    }
  else
    {
#ifdef DEBUG
      std::cerr << "Handler::send_header_result: " << e.message() << std::endl;
#endif
    }
}

template<typename CmdType>
void
Handler<CmdType>::send_result(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "Handler::send_result" << std::endl;
#endif
      // read result from the output message queue
      typename CmdType::return_type result;

      // Send the result to the client. The connection::async_write()
      // function will automatically serialize the data structure for
      // us.
      sesh->conn->async_write(result,
			boost::bind(&Handler<CmdType>::send_header_result, this,
				    boost::asio::placeholders::error, sesh, cmd));      
    }
  else
    {
#ifdef DEBUG
      std::cerr << "Handler::send_result: " << e.message() << std::endl;
#endif
    }
}


template<typename CmdType>
void
Handler<CmdType>::send_eof(const boost::system::error_code& e, SessionMsgPtr sesh)
{
  if (!e)
    {
 #ifdef DEBUG
      std::cerr << "Handler::send_eof" << std::endl;
#endif
      std::string str_eof = HEADER_EOF;
      sesh->conn->async_write(str_eof,
			boost::bind(&Handler<CmdType>::handle_finalize, this,
				    boost::asio::placeholders::error, sesh));      
    }
  else
    {
#ifdef DEBUG
      std::cerr << "Handler::send_result: " << e.message() << std::endl;
#endif
    }
}



template<typename CmdType>
void
Handler<CmdType>::handle_session(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {

#if defined(DEBUG)
      //std::cerr << "in handle session with mess = " << sesh->mess << std::endl;
#endif //DEBUG

      // after processing the message, check whether it's the last
      // one. PrimitiveCommandType and OptCommandType should always
      // return STOP independently from the message, while
      // DynamicCommandType check whether the flag LAST was turned on
      // in the message.
      if (cmd->continues(sesh->mess))
	{
	  std::cerr << "Continue, going to read messages"  << std::endl;
	  sesh->conn->async_read(sesh->mess,
				 boost::bind(&Handler<CmdType>::do_local_job, this,
					     boost::asio::placeholders::error, sesh, cmd)
			   );
	}
      else
	{
	  std::cerr << "Go to send_eof"  << std::endl;
	  send_eof(e, sesh);
	}
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "Handler::handle_session: " << e.message() << std::endl;
#endif
    }
}



template<typename CmdType>
void
Handler<CmdType>::handle_finalize(const boost::system::error_code& e, SessionMsgPtr /* sesh */)
{
  // Nothing to do. The socket will be closed automatically when the last
  // reference to the connection object goes away.
#ifdef DEBUG
  std::cerr << "Handler::handle_finalize: " << std::endl;
#endif
  if (e)
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "Handler::handle_finalize: " << e.message() << std::endl;
#endif
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
