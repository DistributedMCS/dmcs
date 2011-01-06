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
 * @file   RelSatSolver.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  4 17:27:21 2011
 * 
 * @brief  
 * 
 * 
 */


#include "solver/RelSatSolver.h"

namespace dmcs {

RelSatSolver::RelSatSolver(const ContextPtr& context_,
			   const TheoryPtr& theory_, 
			   const QueryPlanPtr& query_plan_,
			   const SignatureVecPtr& global_sigs_)
  : context(context_),
    theory(theory_), 
    query_plan(query_plan_),
    global_sigs(global_sigs_),
    xInstance(std::cerr),
    xSATSolver(&xInstance, std::cerr, context->getSystemSize()),
    sig_size(0)
{
  xSATSolver.setSystemSize(context->getSystemSize());
  std::cerr << "RelSatSolver::constructor, system_size = " << xSATSolver.getSystemSize() << std::endl;
}



int
RelSatSolver::solve(const TheoryPtr& theory, std::size_t sig_size)
{
}



///@todo: move this to an appropriate place. This is code duplication with DMCS stuff.
std::size_t
RelSatSolver::updateGuessingSignature(SignaturePtr& guessing_sig, 
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
#ifdef DEBUG
	  std::cerr << "Bit " << i << "is on" << std::endl;
#endif // DEBUG
	  SignatureByLocal::const_iterator neighbor_it = neighbor_loc.find(i);
	  
	  // the neighbor's V must be set up properly
	  assert(neighbor_it != neighbor_loc.end());

#ifdef DEBUG
	  std::cerr << "want to insert " << neighbor_it->sym << std::endl;
#endif // DEBUG
	  
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

#ifdef DEBUG
	      std::cerr << neighbor_it->sym << "inserted" << std::endl;
#endif // DEBUG
	    }
	}
    }

  return guessing_sig_local_id;
}



SignaturePtr 
RelSatSolver::createGuessingSignature(const BeliefStatePtr& V, const SignaturePtr& my_sig)
{
  SignaturePtr guessing_sig(new Signature);

  // local id in guessing_sig will start from my signature's size + 1
  std::size_t guessing_sig_local_id = my_sig->size() + 1;

  const SignatureBySym& my_sig_sym = boost::get<Tag::Sym>(*my_sig);

  const NeighborListPtr& neighbors = context->getNeighbors();

  for (NeighborList::const_iterator n_it = neighbors->begin(); n_it != neighbors->end(); ++n_it)
    {
      NeighborPtr nb = *n_it;
      const std::size_t neighbor_id = nb->neighbor_id;
      const BeliefSet neighbor_V = (*V)[neighbor_id - 1];
      const Signature& neighbor_sig = *((*global_sigs)[neighbor_id - 1]);

#ifdef DEBUG
      std::cerr << "Interface variable of neighbor[" << nb->neighbor_id <<"]: " << neighbor_V << std::endl;
#endif

      guessing_sig_local_id = updateGuessingSignature(guessing_sig,
						      my_sig_sym,
						      neighbor_sig,
						      neighbor_V,
						      guessing_sig_local_id);
    }
      
#ifdef DEBUG
    std::cerr << "Guessing signature: " << *guessing_sig << std::endl;
#endif

    return guessing_sig;
}



int
RelSatSolver::solve()
{
  std::cerr << "in RelSatSolver::solve(), system_size = " << xSATSolver.getSystemSize() << std::endl;
  // read the query from QueryMessageQueue

  // from this we get (invoker), (conflict)

  // then we can create guessing signature based on the interface from
  // query_plan(invoker, myself)

  std::size_t system_size = context->getSystemSize();
  //  std::size_t invoker = 1;
  BeliefStatePtr conflict(new BeliefState(system_size, BeliefSet()));

  std::size_t my_id = context->getContextID();
  BeliefStatePtr localV;

  // if (invoker == 0)
  if (my_id == 1)
    {
      localV = query_plan->getGlobalV();
    }
  else
    {
      //localV = query_plan->getInterface(invoker, my_id);
      localV = query_plan->getInterface(1, my_id);
    }

  // compute the size of the mixed signature, just once.
  if (sig_size == 0)
    {
      const SignaturePtr& sig = context->getSignature();
      const SignaturePtr& gsig = createGuessingSignature(localV, sig);
      ProxySignatureByLocalPtr mixed_sig(new ProxySignatureByLocal(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*gsig)));
      sig_size = mixed_sig->size();
      xSATSolver.setMixedSignature(mixed_sig);

#ifdef DEBUG
      std::cerr << "Mixed sig size = " << sig_size << std::endl;
#endif
    }

  if (!xInstance.hasTheory())
    {
      xInstance.readTheory(theory, sig_size);
    }

  //return solve(theory, sig_size);


  std::cerr << "Going to call esolve, system_size = " << xSATSolver.getSystemSize() << std::endl;
  relsat_enum eResult = xSATSolver.eSolve();

  return 10;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
