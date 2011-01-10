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
 * @file   Server.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Wed Nov  4 14:25:32 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef SERVER_H
#define SERVER_H

//#include "dmcs/BaseDMCS.h"

#include "network/BaseHandler.h"
#include "network/connection.hpp"
#include "network/Session.h"

#include "mcs/BeliefState.h"
#include "mcs/Rule.h"
#include "mcs/Theory.h"
#include "dmcs/Context.h"
#include "dmcs/CommandType.h"
#include "dmcs/CommandTypeFactory.h"


#include <algorithm>
#include <cstdlib>
#include <list>
#include <set>

#include <boost/shared_ptr.hpp>


namespace dmcs {



struct BaseServer
{};


/**
 * @brief
 * 
 */
class Server : public BaseServer
{
public:
  Server(CommandTypeFactoryPtr& ctf_, boost::asio::io_service& io_service,
	 const boost::asio::ip::tcp::endpoint& endpoint);

  void 
  handle_accept(const boost::system::error_code& e, connection_ptr conn);

  void
  dispatch_header(const boost::system::error_code& e, connection_ptr conn);

  void 
  handle_finalize(const boost::system::error_code& e, connection_ptr conn);


private:
  CommandTypeFactoryPtr ctf;
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::string header;
  BaseHandler* handler;
};

typedef boost::shared_ptr<BaseServer> ServerPtr;

} // namespace dmcs


#endif // SERVER_H

// Local Variables:
// mode: C++
// End:
