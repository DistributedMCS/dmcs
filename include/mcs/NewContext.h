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
 * @file   NewContext.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  21 17:43:20 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_CONTEXT_H
#define NEW_CONTEXT_H

#include "dmcs/Instantiator.h"
#include "mcs/BeliefTable.h"

struct NewContext 
{
  std::size_t ctx_id;

  InstantiatorPtr inst;

  BridgeRuleTablePtr bridge_rules;
  BeliefTablePtr local_signature;

  BeliefTableVec neighbors_interfaces;
  NewNeighborVec neighbors;

  // ?? should the context have a method to read off the query plan
  // or every data member of the context is initialized during construction time??

  NewContext(std::size_t cid)
    : ctx_id(cid)
  { }
};

#endif // NEW_CONTEXT_H
  
// Local Variables:
// mode: C++
// End:
