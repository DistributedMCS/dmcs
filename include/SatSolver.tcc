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
 * @file   SatSolver.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:33:26 2009
 * 
 * @brief  
 * 
 * 
 */

#if !defined(_SAT_SOLVER_TCC)
#define _SAT_SOLVER_TCC

#include "BaseBuilder.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ParserDirector.h"

#include <exception>


namespace dmcs {


template <typename Builder, typename Parser, typename ParserGrammar>
SatSolver<Builder, Parser, ParserGrammar>::SatSolver(Process& p)
  : proc(p)
{ }


typedef std::list<Signature::const_iterator> SignatureIterators;

template <typename Builder, typename Parser, typename ParserGrammar>
void
SatSolver<Builder, Parser, ParserGrammar>::solve(SignatureByLocal& context_signature,
						 BeliefStateListPtr& belief_states,
						 const TheoryPtr& theory
						 )
{
  int retcode = -1;
  
  try
    {

      ///@todo remove , replaced by ProxySignatureByLocal



      //      
      // first, go through the neighbors and setup the additional Signature from V
      //



      const SignaturePtr& sig = context.getSignature();

      SignatureIterators insert_iterators;

      
#ifdef DEBUG
      std::cerr << "Original signature: " << *sig << std::endl;
#endif
      
      std::size_t my_id = context.getContextID();
      const QueryPlanPtr& query_plan = context.getQueryPlan();
      
      const NeighborsPtr& neighbors = query_plan->getNeighbors(my_id);



      for (Neighbors::const_iterator n_it = neighbors->begin();
	   n_it != neighbors->end();
	   ++n_it)
	{
	  const BeliefSet neighbor_V = (*V)[*n_it - 1];
	  
#ifdef DEBUG
	  std::cerr << "Interface variable of neighbor[" << *n_it <<"]: " << neighbor_V << std::endl;
#endif
	  
	  const Signature& neighbor_sig = query_plan->getSignature(*n_it);
	  const SignatureByLocal& neighbor_loc = boost::get<Tag::Local>(neighbor_sig);
	  
	  // setup local signature for neighbors: this way we can translate
	  // SAT models back to belief states in case we do not
	  // reference them in the bridge rules
	  //for (std::size_t i = 1; i < sizeof(neighbor_V)*8; ++i)
	  for (std::size_t i = 1; i <= neighbor_sig.size(); ++i) // at most sig-size bits are allowed
	    {
	      if (testBeliefSet(neighbor_V, i))
		{
		  SignatureByLocal::const_iterator neighbor_it = neighbor_loc.find(i);

		  // the neighbor's V must be set up properly
		  assert(neighbor_it != neighbor_loc.end());

		  std::size_t local_id_here = sig->size()+1; // compute new local id for i'th bit
		  
		  // add new symbol for neighbor
		  Symbol sym(neighbor_it->sym, neighbor_it->ctxId, local_id_here, neighbor_it->origId);
		  std::pair<Signature::iterator, bool> sp = sig->insert(sym);
		  
		  // only add them if it was not already included
		  // during bridge rule parsing
		  if (sp.second)
		    {
		      insert_iterators.push_back(sp.first);
		    }
		}
	    }
	}
      
#ifdef DEBUG
      std::cerr << "Updated signature: " << *sig << std::endl;
#endif

      ///@todo end remove

      //
      // now send the theory to the SAT solver using the adapted Signature
      //
     
      proc.spawn();
      
      Builder builder(proc.getOutput());
      builder.visitTheory(theory, context_signature->size());

#ifdef DEBUG
      Builder evil_builder(std::cerr);
      evil_builder.visitTheory(theory, sig->size());
#endif
      
      proc.endoffile();
      
      //
      // parse result and set belief states
      //
      
#if defined(DEBUG)
      std::cerr << "Parsing the models..." << std::endl;
#endif // DEBUG
      
      Parser model_builder(context_signature, belief_states);
      
      ParserDirector<ParserGrammar> parser_director;
      parser_director.setBuilder(&model_builder);
      parser_director.parse(proc.getInput());      

      //
      // close the connection to the SAT solver
      //

      retcode = proc.close();

      //
      // restore Signature
      //
      
      ///@todo delete
#ifdef DEBUG      
      std::cerr << "Erasing..." << std::endl;
#endif
      
      for (SignatureIterators::const_iterator s_it = insert_iterators.begin();
	   s_it != insert_iterators.end(); 
	   ++s_it)
	{
	  sig->erase(*s_it);
	}
      
#ifdef DEBUG
      std::cerr << "Restored signature: " << *sig << std::endl;
#endif
    }
  ///@todo end delete
  catch (std::ios_base::failure& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
      throw e;
    }
  catch(std::exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
      throw e;
    }
}
  
} // namespace dmcs

#endif // _SAT_SOLVER_TCC

// Local Variables:
// mode: C++
// End:
