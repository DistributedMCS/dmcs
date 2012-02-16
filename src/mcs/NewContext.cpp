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
 * @file   NewContext.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Feb  16 20:57:26 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/NewContext.h"
#include "mcs/ForwardMessage.h"

namespace dmcs {

NewContext::NewContext(std::size_t cid,
		       InstantiatorPtr i,
		       BridgeRuleTablePtr br,
		       BeliefTablePtr ex_sig,
		       NewNeighborVec nbs)
  : ctx_id(cid),
    inst(i),
    bridge_rules(br),
    export_signature(ex_sig),
    neighbors(nbs)
{ }


void
NewContext::operator()(NewConcurrentMessageDispatcherPtr md,
		       NewJoinerDispatcherPtr jd)
{
  while (1)
    {
      // listen to the REQUEST_MQ
      int timeout = 0;

      ForwardMessage* fwd_mess = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id, timeout);
      // call joiner
      // call bridge rule evaluator
      // send heads to evaluator_in_mq
      // wait at evaluator_out_mq and combine
    }
}


void
NewContext::combine()
{
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
