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
 * @file   OutputThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:25:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef OUTPUT_THREAD_H
#define OUTPUT_THREAD_H

#include "dmcs/OutputNotification.h"
#include "mcs/BeliefState.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/connection.hpp"
#include "solver/Conflict.h"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcs {

struct PbsResult
{
  PartialBeliefState* pbs;
  std::size_t session_id;

  PbsResult(PartialBeliefState* p, std::size_t sid)
    : pbs(p), session_id(sid)
  {
  }
};

typedef std::list<PbsResult> PbsResultList;
typedef boost::shared_ptr<PbsResultList> PbsResultListPtr;

inline bool
my_compare(const PbsResult& pbs1, const PbsResult& pbs2)
{
  if (*(pbs1.pbs) < *(pbs2.pbs))
    {
      return true;
    }
  else if (*(pbs1.pbs) == *(pbs2.pbs))
    {
      if (pbs1.session_id > pbs2.session_id)
	{
	  return true;
	}
    }
  
  return false;
}

inline bool
operator== (const PbsResult& pbs1, const PbsResult& pbs2)
{
  return (*(pbs1.pbs) == *(pbs2.pbs));
}


class OutputThread
{
public:
  OutputThread(std::size_t i, std::size_t p);

  virtual
  ~OutputThread();

  void
  operator()(connection_ptr c,
	     MessagingGatewayBC* m,
	     ConcurrentMessageQueue* hon);

private:
  bool
  wait_for_trigger(ConcurrentMessageQueue* handler_output_notif);

  bool
  collect_output(MessagingGatewayBC* mg, 
		 //PartialBeliefStateVecPtr& res, 
		 //VecSizeTPtr& res_sid, 
		 PbsResultListPtr& res,
		 std::string& header);

  void
  write_result(connection_ptr conn,
	       //PartialBeliefStateVecPtr& res,
	       //VecSizeTPtr& res_sid,
	       PbsResultListPtr& res,
	       const std::string& header);

  void
  handle_written_header(connection_ptr conn,
			//PartialBeliefStateVecPtr& res,
			//VecSizeTPtr& res_sid);
			PbsResultListPtr& res);

private:
  std::size_t pack_size;             // Number of models the invoker expects to get
  std::size_t left_2_send;           // Number of models left to send
  bool        collecting;            // A flag to determine whether we are in collecting mode or not 
                                     // (if yes then we don't want to wait for any trigger)
  bool        eof_left;
  bool        end_of_everything;
  std::size_t invoker;
  std::size_t parent_session_id;     // used to filter out old models
  std::size_t port;
};

typedef boost::shared_ptr<OutputThread> OutputThreadPtr;

} // namespace dmcs


#endif // OUTPUT_THREAD_H

// Local Variables:
// mode: C++
// End:
