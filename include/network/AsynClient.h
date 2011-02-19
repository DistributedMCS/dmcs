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
 * @file   AsynClient.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  10 14:15:24 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef ASYN_CLIENT_H
#define ASYN_CLIENT_H

#include "network/BaseClient.h"
#include "network/ConcurrentMessageQueueFactory.h"

namespace dmcs {

/**
 * @brief
 */
template <typename ForwardMessType, typename BackwardMessType>
class AsynClient // : public BaseClient
{
public:
  AsynClient(boost::asio::io_service& io_service,
	     boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	     const std::string& h,
	     ForwardMessType& fm);


  typedef void (*CallbackFun)(BackwardMessType&);

  void
  setCallback(CallbackFun f)
  {
    callback = f;
  }


  bool
  next(ForwardMessType&);

  void
  terminate();


private:
  void 
  send_header(const boost::system::error_code& error,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	      connection_ptr conn,
	      boost::shared_ptr<std::string> my_header,
	      boost::shared_ptr<ForwardMessType> mess);

  void
  send_message(const boost::system::error_code& error,
	       connection_ptr conn,
	       boost::shared_ptr<std::string> my_header,
	       boost::shared_ptr<ForwardMessType> mess);

  void
  read_header(const boost::system::error_code& error,
	      connection_ptr conn,
	      boost::shared_ptr<std::string> my_header,
	      boost::shared_ptr<ForwardMessType> mess);

  void 
  handle_read_header(const boost::system::error_code& error,
		     connection_ptr conn,
		     boost::shared_ptr<std::string> my_header,
		     boost::shared_ptr<ForwardMessType> mess);

  void 
  handle_read_answer(const boost::system::error_code& error,
		     connection_ptr conn,
		     boost::shared_ptr<std::string> my_header,
		     boost::shared_ptr<ForwardMessType> mess,
		     boost::shared_ptr<BackwardMessType> result);

  void 
  finalize(const boost::system::error_code& error,
	   connection_ptr /* conn */,
	   boost::shared_ptr<std::string> my_header);


  connection_ptr the_conn;
  CallbackFun    callback;
};

} // namespace dmcs

#include "network/AsynClient.tcc"

#endif // ASYN_CLIENT_H

// Local Variables:
// mode: C++
// End:
