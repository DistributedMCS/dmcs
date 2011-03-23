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
 * @file   Handler.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  10 10:56:32 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_HANDLER_H
#define BASE_HANDLER_H

#include "network/Session.h"

#include <boost/shared_ptr.hpp>

namespace dmcs {


template<typename CmdType>
class BaseHandler
{
public:

  typedef Session<typename CmdType::input_type> SessionMsg;
  typedef typename boost::shared_ptr<SessionMsg> SessionMsgPtr;
  typedef typename boost::shared_ptr<CmdType> CmdTypePtr;

  typedef typename boost::shared_ptr<BaseHandler<CmdType> > HandlerPtr;


  inline void
  start(HandlerPtr hdl, SessionMsgPtr sesh, CmdTypePtr cmd)
  {
    assert(hdl.get() == this);

    // read and process this message
    sesh->conn->async_read(sesh->mess,
			   boost::bind(&BaseHandler<CmdType>::do_local_job, this,
				       boost::asio::placeholders::error,
				       hdl,
				       sesh,
				       cmd,
				       true) // first request
			   );
  }


  /*  virtual */void
  do_local_job(const boost::system::error_code& e,
	       HandlerPtr hdl,
	       SessionMsgPtr sesh,
	       CmdTypePtr cmd,
	       bool first_call)
  { }
	       

};


} // namespace dmcs

#endif // BASE_HANDLER_H

// Local Variables:
// mode: C++
// End:
