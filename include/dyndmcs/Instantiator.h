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
 * @file   Instantiator.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Sep  23 14:18:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATOR_H
#define INSTANTIATOR_H

#include <boost/shared_ptr.hpp>

#include "dyndmcs/InstantiateForwardMessage.h"
#include "dyndmcs/InstantiateBackwardMessage.h"
#include "mcs/Rule.h"

namespace dmcs {

class Instantiator
{
public:

#ifdef DMCS_STATS_INFO
#else
  typedef bool dynmcs_value_type;
  typedef bool dynmcs_return_type;
#endif

  Instantiator(std::size_t ctx_id_, BridgeRulesPtr schematic_bridge_rules_, 
	       BridgeRulesPtr bridge_rules_, SignatureVecPtr global_sigs_,
	       NeighborListPtr context_info_)
    : ctx_id(ctx_id_), schematic_bridge_rules(schematic_bridge_rules_), 
      bridge_rules(bridge_rules_), global_sigs(global_sigs_),
      context_info(context_info_)
  { }

  InstantiateBackwardMessagePtr
  instantiate(InstantiateForwardMessage& mess);

private:
  BridgeRulesPtr schematic_bridge_rules;
  BridgeRulesPtr bridge_rules;
  SignatureVecPtr global_sigs;
  NeighborListPtr context_info;
  std::size_t ctx_id;
};

typedef boost::shared_ptr<Instantiator> InstantiatorPtr;

} // namespace dmcs

#endif // INSTANTIATOR_H

#include "dyndmcs/Instantiator.tcc"

// Local Variables:
// mode: C++
// End:
