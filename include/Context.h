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
 * @file   Context.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Wed Nov  4 11:20:38 2009
 * 
 * @brief  
 * 
 * 
 */


#if !defined(CONTEXT_H)
#define CONTEXT_H


#include "Theory.h"
#include "BeliefState.h"
#include "QueryPlan.h"
#include "Rule.h"
#include "Signature.h"

#include <vector>


namespace dmcs {

/**
 * @brief a Context has an ID, a size, a signature, some neighbors, and KB + BR
 * 
 */
class Context
{
public:
  Context(std::size_t id_, std::size_t system_size_,
	  SignaturePtr& signature_,
	  QueryPlanPtr& query_plan_,
	  RulesPtr& local_kb_,
	  BridgeRulesPtr& bridgerules_,
	  NeighborListPtr& neighbor_list_)
    : id(id_),
      system_size(system_size_),
      signature(signature_),
      query_plan(query_plan_),
      local_kb(local_kb_),
      bridgerules(bridgerules_),
      neighbor_list(neighbor_list_)
  { }

  std::size_t
  getContextID() const
  {
    return id;
  }

  std::size_t 
  getSystemSize() const
  {
    return system_size;
  }

  const QueryPlanPtr&
  getQueryPlan() const
  {
    return query_plan;
  }

  const NeighborListPtr
  getNeighbors() const
  {
    return neighbor_list;
  }

  const SignaturePtr&
  getSignature() const
  {
    return signature;
  }

  const RulesPtr&
  getLocalKB() const
  {
    return local_kb;
  }

  const BridgeRulesPtr&
  getBridgeRules() const
  {
    return bridgerules;
  }

private:
  std::size_t id;
  std::size_t system_size;
  SignaturePtr signature;
  QueryPlanPtr query_plan;
  RulesPtr local_kb;
  BridgeRulesPtr bridgerules;
  NeighborListPtr neighbor_list;
};

typedef boost::shared_ptr<Context> ContextPtr;
typedef std::vector<ContextPtr> Contexts;
typedef boost::shared_ptr<Contexts> ContextsPtr;

} // namespace dmcs

#endif // CONTEXT_H

// Local Variables:
// mode: C++
// End:
