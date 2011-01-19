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
 * @file   BaseDMCS.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun May  2 16:42:53 2010
 * 
 * @brief  Base DMCS methods.
 * 
 * 
 */


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "dmcs/BaseDMCS.h"
#include "process/ClaspProcess.h"
#include "dmcs/StatsInfo.h"
#include "dmcs/Log.h"

namespace dmcs {


BaseDMCS::BaseDMCS(const ContextPtr& c, const TheoryPtr& t, const SignatureVecPtr& s)
  : ctx(c),
    theory(t),
#ifdef DMCS_STATS_INFO
    global_sigs(s),
    sis(new StatsInfos)
#else
    global_sigs(s)
#endif // DMCS_STATS_INFO
{ }


BaseDMCS::~BaseDMCS()
{ }



std::size_t
BaseDMCS::updateGuessingSignature(SignaturePtr& guessing_sig, 
			const SignatureBySym& my_sig_sym,
			const Signature& neighbor_sig,
			const BeliefSet& neighbor_V,
			std::size_t guessing_sig_local_id)
{
  const SignatureByLocal& neighbor_loc = boost::get<Tag::Local>(neighbor_sig);
      
  // setup local signature for neighbors: this way we can translate
  // SAT models back to belief states in case we do not
  // reference them in the bridge rules
  //for (std::size_t i = 1; i < sizeof(neighbor_V)*8; ++i)
  for (std::size_t i = 1; i <= neighbor_sig.size(); ++i) // at most sig-size bits are allowed
    {
      if (testBeliefSet(neighbor_V, i))
	{
	  DMCS_LOG_TRACE("Bit " << i << "is on");

	  SignatureByLocal::const_iterator neighbor_it = neighbor_loc.find(i);
	  
	  // the neighbor's V must be set up properly
	  assert(neighbor_it != neighbor_loc.end());

	  DMCS_LOG_DEBUG("want to insert " << neighbor_it->sym);
	  
	  SignatureBySym::const_iterator my_it = my_sig_sym.find(neighbor_it->sym);
	  
	  // only add to guessing_sig if this atom is not in my_sig,
	  // i.e., it's in the neighbor's interface but does not show
	  // up in my bridge rules
	  if (my_it == my_sig_sym.end())
	    {
	      // add new symbol for neighbor
	      Symbol sym(neighbor_it->sym, neighbor_it->ctxId, guessing_sig_local_id, neighbor_it->origId);
	      guessing_sig->insert(sym);
	      guessing_sig_local_id++;

	      DMCS_LOG_DEBUG(neighbor_it->sym << "inserted");
	    }
	}
    }

  return guessing_sig_local_id;
}



SignaturePtr
BaseDMCS::createGuessingSignature(const BeliefStatePtr& V, const SignaturePtr& my_sig)
{
  SignaturePtr guessing_sig(new Signature);

  // local id in guessing_sig will start from my signature's size + 1
  std::size_t guessing_sig_local_id = my_sig->size() + 1;

  const SignatureBySym& my_sig_sym = boost::get<Tag::Sym>(*my_sig);

  const NeighborListPtr& neighbors = ctx->getNeighbors();

  for (NeighborList::const_iterator n_it = neighbors->begin(); n_it != neighbors->end(); ++n_it)
    {
      NeighborPtr nb = *n_it;
      const std::size_t neighbor_id = nb->neighbor_id;
      const BeliefSet neighbor_V = (*V)[neighbor_id - 1];
      const Signature& neighbor_sig = *((*global_sigs)[neighbor_id - 1]);

      DMCS_LOG_DEBUG("Interface variable of neighbor[" << nb->neighbor_id <<"]: " << neighbor_V);

      guessing_sig_local_id = updateGuessingSignature(guessing_sig,
						      my_sig_sym,
						      neighbor_sig,
						      neighbor_V,
						      guessing_sig_local_id);
    }
      
  DMCS_LOG_DEBUG("Guessing signature: " << *guessing_sig);

  return guessing_sig;
}



BeliefStateListPtr
BaseDMCS::localSolve(const ProxySignatureByLocal& sig, std::size_t system_size)
{
  DMCS_LOG_DEBUG("Starting local solve...");

  BeliefStateListPtr local_belief_states(new BeliefStateList);

  ///@todo TK: fix the system size, a context should not depend on the whole MCS and the query plan
  ClaspResultBuilder<ClaspResultGrammar> crb(sig, local_belief_states, system_size);

  ClaspProcess cp;
  cp.addOption("-n 0");
  boost::shared_ptr<BaseSolver> solver(cp.createSolver(&crb));

  ///@todo 
  solver->solve(theory, sig.size());

  DMCS_LOG_DEBUG("Got " << local_belief_states->size() << " answers from solver");
  DMCS_LOG_DEBUG("Local belief states from localsolve():");
  DMCS_LOG_DEBUG(*local_belief_states);

  return local_belief_states;
}



/// methods only needed for providing statistic information
#ifdef DMCS_STATS_INFO

void
BaseDMCS::initStatsInfos(std::size_t system_size)
{
  // initialize the statistic information
  sis->clear();
  
  for (std::size_t i = 0; i < system_size; ++i)
    {
      TimeDuration local_i(0, 0, 0, 0);
      TimeDuration combine_i(0, 0, 0, 0);
      TimeDuration project_i(0, 0, 0, 0);
      
      Info info_local(local_i, 0);
      Info info_combine(combine_i, 0);
      Info info_project(project_i, 0);
      
      TransferTimesPtr tf_i(new TransferTimes);
      
      StatsInfo si(info_local, info_combine, info_project, tf_i);
      
      sis->push_back(si);
    }

  DMCS_LOG_DEBUG("Initialization of the statistic information: " << *sis);
}

#endif // DMCS_STATS_INFO


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
