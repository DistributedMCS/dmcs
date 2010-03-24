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
 * @file   ContextGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sat Jan  16 18:16:30 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef CONTEXT_GENERATOR_H
#define CONTEXT_GENERATOR_H

#include "Context.h"
#include "QueryPlan.h"


namespace dmcs { namespace generator {

class ContextGenerator
{
public:
  ContextGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_,
		   int no_bridge_rules_, int no_atoms_)
    : contexts(contexts_),
      query_plan(query_plan_),
      no_bridge_rules(no_bridge_rules_),
      no_atoms(no_atoms_)
  { }

  void
  generate();

  //  virtual void
  //  generate_local_kb(const ContextPtr& context) = 0;

  void
  generate_local_kb(const ContextPtr& context);
  
  void 
  generate_bridge_rule(const ContextPtr& context);

  void 
  generate_bridge_rules(const ContextPtr& context);

protected:
  int
  sign();

  bool
  cover_neighbors(const ContextPtr& context);

protected:
  ContextsPtr contexts;
  QueryPlanPtr& query_plan;
  int no_bridge_rules;
  int no_atoms;
};

  } // namespace generator
} // namespace dmcs

#endif // CONTEXT_GENERATOR_H

// Local Variables:
// mode: C++
// End:
