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
 * @file   BridgeRuleEvaluator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  23 16:30:45 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefStateOffset.h"
#include "mcs/BridgeRuleEvaluator.h"

namespace dmcs {


bool
satisfied(const BridgeRule& r, 
	  const NewBeliefState* input,
	  const std::vector<std::size_t>& starting_offset)
{
  for (Tuple::const_iterator it = r.body.begin();
       it != r.body.end(); ++it)
    {
      assert (ID::isBelief(*it));
      std::size_t ctx_id = it->contextID();
      IDAddress address = it->address;

      std::size_t global_address = address + starting_offset[ctx_id];

      if (it->isNaf())
	{
	  if (input->test(global_address) == NewBeliefState::DMCS_TRUE)
	    {
	      return false;
	    }
	}
      else
	{
	  if (input->test(global_address) != NewBeliefState::DMCS_TRUE)
	    {
	      return false;
	    }
	}
    }

  return true;
}


Heads*
evaluate_bridge_rules(const BridgeRuleTablePtr& brtab,
		      const NewBeliefState* input, 
		      const std::vector<std::size_t>& starting_offset)
{
  NewBeliefState* h = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
					 BeliefStateOffset::instance()->SIZE_BS());
  
  std::pair<BridgeRuleTable::AddressIterator, BridgeRuleTable::AddressIterator> iters = brtab->getAllByAddress();

  for (BridgeRuleTable::AddressIterator it = iters.first; it != iters.second; ++it)
    {
      const BridgeRule& r = *it;
      if (satisfied(r, input, starting_offset))
	{
	  std::size_t ctx_id = r.head.contextID();
	  IDAddress address = r.head.address;
	  std::size_t global_address = address + starting_offset[ctx_id];
	  h->set(global_address);
	}
    }

  HeadsPlusBeliefState* heads = new HeadsPlusBeliefState(h, input);
  return heads;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
