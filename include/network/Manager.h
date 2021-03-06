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
 * @file   Manager.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Oct  25 22:28:26 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "network/connection.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

class Manager
{
public:
  Manager(boost::asio::io_service& i,
	  const boost::asio::ip::tcp::endpoint& endpoint,
	  std::size_t system_size);

  void
  handle_accept(const boost::system::error_code& e, 
		connection_ptr conn);

  void
  handle_read_header(const boost::system::error_code& e, 
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header);

  void
  trigger_2nd_phase();

  void
  wait_termination(const boost::system::error_code& e, 
		   connection_ptr conn);

  void
  handle_finalize(const boost::system::error_code& e, 
		  connection_ptr conn,
		  boost::shared_ptr<std::string> header);

  void
  close_all_connections();

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::vector<connection_ptr> connections_vec;
  std::size_t system_size;
  std::size_t count_terminations;
  boost::mutex mtx;
};

} // namespace dmcs

#endif // __MANAGER_H__


// Local Variables:
// mode: C++
// End:
