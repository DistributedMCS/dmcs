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
 * @file   CommandTypeFactory.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Sep 29 09:21:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef COMMAND_TYPE_FACTORY_H
#define COMMAND_TYPE_FACTORY_H


#include "dyndmcs/DynamicConfiguration.h"
#include "dyndmcs/Match.h"
#include "dyndmcs/DynamicCommandType.h"
#include "dyndmcs/InstantiatorCommandType.h"

#include "dmcs/OptCommandType.h"
#include "dmcs/PrimitiveCommandType.h"
#include "dmcs/StreamingCommandType.h"

#include "dmcs/Message.h"
#include "dmcs/ProgramOptions.h"
#include "dmcs/Neighbor.h"

#include <boost/shared_ptr.hpp>


namespace dmcs {

class CommandTypeFactory
{
public:
  CommandTypeFactory(std::size_t ctx_id_,
		     std::size_t system_size_,
		     bool cd,
		     RulesPtr& local_kb_,
		     NeighborListPtr& neighbor_list_,
		     BridgeRulesPtr& schematic_bridge_rules_,
		     BridgeRulesPtr& bridge_rules_,
		     NeighborListPtr& context_info_,
		     MatchTablePtr& mt_,
		     CountVecPtr& sba_count_,
		     std::size_t limit_answers_,
		     std::size_t limit_bind_rules_,
		     std::size_t heuristics_,
		     const std::string& prefix_,
		     SignatureVecPtr& global_sigs_,
		     VecSizeTPtr& oss,
		     SignaturePtr& sig_,
		     QueryPlanPtr& query_plan_,
		     TheoryPtr& loopFormula_,
		     std::size_t mqs,
		     std::size_t buf_count_ = 100)
    : ctx_id(ctx_id_),
      system_size(system_size_),
      conflicts_driven(cd),
      schematic_bridge_rules(schematic_bridge_rules_),
      bridge_rules(bridge_rules_),
      context_info(context_info_),
      mt(mt_),
      sba_count(sba_count_),
      limit_answers(limit_answers_),
      limit_bind_rules(limit_bind_rules_),
      heuristics(heuristics_),
      prefix(prefix_),
      global_sigs(global_sigs_),
      orig_sigs_size(oss),
      sig(sig_),
      query_plan(query_plan_),
      loopFormula(loopFormula_),
      mq_size(mqs),
      ctx(new Context(ctx_id_, system_size_, sig_, local_kb_, bridge_rules_, neighbor_list_)),
      buf_count(buf_count_)
  { }

  template<typename aCommandTypePtr>
  aCommandTypePtr
  create();

  ContextPtr
  getContext()
  {
    return ctx;
  }

  TheoryPtr
  getTheory()
  {
    return loopFormula;
  }

  QueryPlanPtr
  getQueryPlan()
  {
    return query_plan;
  }

  std::size_t
  getMQSize()
  {
    return mq_size;
  }

private:
  std::size_t ctx_id;
  std::size_t system_size;
  bool conflicts_driven;
  BridgeRulesPtr schematic_bridge_rules;
  BridgeRulesPtr bridge_rules;
  NeighborListPtr context_info;
  MatchTablePtr mt;
  CountVecPtr sba_count;
  std::size_t limit_answers;
  std::size_t limit_bind_rules;
  std::size_t heuristics;
  const std::string& prefix;
  SignatureVecPtr global_sigs;
  VecSizeTPtr orig_sigs_size;
  SignaturePtr sig;
  QueryPlanPtr query_plan;
  TheoryPtr loopFormula;
  ContextPtr ctx;
  std::size_t mq_size;
  std::size_t buf_count;                    // the number of BeliefState(s) that we
					    // are willing to store in a buffer. This is a
					    // middle solution between having exponential space 
                                            // and total recomputation.
};


typedef boost::shared_ptr<CommandTypeFactory> CommandTypeFactoryPtr;



template<>
inline PrimitiveCommandTypePtr
CommandTypeFactory::create<PrimitiveCommandTypePtr>()
{
  PrimitiveDMCSPtr pri_dmcs(new PrimitiveDMCS(ctx, loopFormula, global_sigs));
  PrimitiveCommandTypePtr cmt_pri_dmcs(new PrimitiveCommandType(pri_dmcs));

  return cmt_pri_dmcs;
}



template<>
inline OptCommandTypePtr
CommandTypeFactory::create<OptCommandTypePtr>()
{
  OptDMCSPtr opt_dmcs(new OptDMCS(ctx, loopFormula, global_sigs, query_plan));
  OptCommandTypePtr cmt_opt_dmcs(new OptCommandType(opt_dmcs));

  return cmt_opt_dmcs;
}



template<>
inline StreamingCommandTypePtr
CommandTypeFactory::create<StreamingCommandTypePtr>()
{
  //StreamingDMCSPtr stm_dmcs(new StreamingDMCS(ctx, loopFormula, global_sigs, query_plan));
  //StreamingCommandTypePtr stm_opt_dmcs(new StreamingCommandType(stm_dmcs));
  StreamingCommandTypePtr stm_opt_dmcs(new StreamingCommandType);

  return stm_opt_dmcs;
}



template<>
inline DynamicCommandTypePtr
CommandTypeFactory::create<DynamicCommandTypePtr>()
{
  DynamicConfigurationPtr dconf(new DynamicConfiguration(ctx_id, schematic_bridge_rules, context_info, mt, sba_count, limit_answers, limit_bind_rules, heuristics, prefix));
  DynamicCommandTypePtr cmt_dyn_conf(new DynamicCommandType(dconf));
  return cmt_dyn_conf;
}



template<>
inline InstantiatorCommandTypePtr
CommandTypeFactory::create<InstantiatorCommandTypePtr>()
{
  InstantiatorPtr inst(new Instantiator(ctx_id, schematic_bridge_rules, bridge_rules, global_sigs, context_info));
  InstantiatorCommandTypePtr cmt_inst(new InstantiatorCommandType(inst));

  return cmt_inst;
}


} // namespace dmcs

#endif // COMMAND_TYPE_FACTORY_H

// Local Variables:
// mode: C++
// End:


