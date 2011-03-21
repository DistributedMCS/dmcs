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
 * @file   Instantiator.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Sep  23 15:08:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <boost/asio.hpp>

#include "dyndmcs/Instantiator.h"
#include "dyndmcs/InstantiatorCommandType.h"

#include "network/Client.h"

namespace dmcs {

InstantiateBackwardMessagePtr
Instantiator::instantiate(InstantiateForwardMessage& mess)
{
  HistoryPtr hist = mess.getHistory();
  BeliefStatePtr interface_vars = mess.getInterfaceVars();
  History::const_iterator hist_it = std::find(hist->begin(), hist->end(), ctx_id);
  if (hist_it != hist->end())
    {
      // This context is already instantiated!
      InstantiateBackwardMessagePtr ibm(new InstantiateBackwardMessage(true, hist, interface_vars));
      return ibm;
    }

  ContextSubstitutionPtr ctx_sub = mess.getCtxSubstitution();
  NeighborListPtr neighbors(new NeighborList);

  DMCS_LOG_TRACE("Context substitution = " << ctx_sub);

  for (BridgeRules::const_iterator r = schematic_bridge_rules->begin(); 
       r != schematic_bridge_rules->end(); ++r)
    {
      // information from r
      HeadPtr head = (*r)->first;
      const PositiveBridgeBody& pbody = getPositiveBody(*r);
      const NegativeBridgeBody& nbody = getNegativeBody(*r);

      DMCS_LOG_DEBUG("Instantiating schematic bridge rule: " << *r);

      // new parts for the instantiated bridge rule
      PositiveBridgeBodyPtr pi_body(new PositiveBridgeBody);
      NegativeBridgeBodyPtr ni_body(new NegativeBridgeBody);
      std::pair<PositiveBridgeBodyPtr, NegativeBridgeBodyPtr> i_body(pi_body, ni_body);
      BridgeRulePtr ri(new BridgeRule(head, i_body));

      instantiate_body<PositiveBridgeBody>(pbody, pi_body, global_sigs, context_info, neighbors, ctx_sub, ctx_id);
      instantiate_body<NegativeBridgeBody>(nbody, ni_body, global_sigs, context_info, neighbors, ctx_sub, ctx_id);
      bridge_rules->push_back(ri);
    }
      
  DMCS_LOG_DEBUG("RESULT: Instantiated bridge rules:");
  DMCS_LOG_DEBUG(bridge_rules);

  // forward the instantiation request to all confirmed neighbors
  DMCS_LOG_DEBUG("Going to instantiate the following neighbors:");
  DMCS_LOG_DEBUG(*neighbors);

  hist->push_back(ctx_id);

  for (NeighborList::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
    {
      NeighborPtr nb = *it;
      std::size_t neighbor_id = nb->neighbor_id;

      // only invoke the not yet visited neighbors
      History::const_iterator hist_it = std::find(hist->begin(), hist->end(), neighbor_id);
      
      if (hist_it == hist->end())
	{
	  boost::asio::io_service io_service;
	  boost::asio::ip::tcp::resolver resolver(io_service);
      
	  boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
	  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
	  boost::asio::ip::tcp::endpoint endpoint = *res_it;
	  
	  DMCS_LOG_DEBUG("Invoking neighbor " << neighbor_id);
	  
	  std::string header = HEADER_REQ_INSTANTIATE;
	  Client<InstantiatorCommandType> client(io_service, res_it, header, mess);
	  
	  io_service.run();
	  
	  InstantiateBackwardMessagePtr result = client.getResult();

	  DMCS_LOG_DEBUG("Answer from neighbor: " << *result);
	  
	  if (result->getStatus() == false)
	    {
	      return result;
	    }

	  hist = result->getHistory();
	  interface_vars = result->getInterfaceVars();
	}
      else
	{
	  DMCS_LOG_DEBUG("Neighbor: " << neighbor_id << " was already instantiated.");
	}
    }
      
  InstantiateBackwardMessagePtr good_result(new InstantiateBackwardMessage(true, hist, interface_vars));
  return good_result;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
