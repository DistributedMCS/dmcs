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

typedef std::list<Signature::const_iterator> SignatureIterators;

template <typename Builder, typename Parser, typename ParserGrammar>
SatSolver<Builder, Parser, ParserGrammar>::SatSolver(Process& p)
  : proc(p)
{ }



template <typename Builder, typename Parser, typename ParserGrammar>
void
SatSolver<Builder, Parser, ParserGrammar>::solve(const Context& context,
						 BeliefStatesPtr& belief_states,
						 const TheoryPtr& theory,
						 const BeliefStatePtr& V)
{
  int retcode = -1;
  
  try
    {
      proc.spawn();
      const SignaturePtr& sig = context.getSignature();
      SignatureIterators insert_iterators;

#ifdef DEBUG
      std::cerr << "Original signature: " << *sig << std::endl;
#endif

      try 
	{
	  Builder builder(proc.getOutput());
	  
	  std::size_t my_id = context.getContextID();
	  const QueryPlanPtr& query_plan = context.getQueryPlan();

	  const NeighborsPtr_& neighbors = query_plan->getNeighbors(my_id);
	  Neighbors_::const_iterator n_it = neighbors->begin();

#ifdef DEBUG
	  std::cerr << "In SatSolver" << std::endl;
	  std::cerr << "Neighbors are: " << std::endl;
	  for (Neighbors_::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
	    {
	      std::cerr << *it << "  ";
	    }
	  std::cerr << std::endl;
#endif
	  

	  for (; n_it != neighbors->end(); ++n_it)
	    {
	      BeliefSet neighbor_V = V.belief_state_ptr->belief_state[*n_it - 1];

#ifdef DEBUG
	      std::cerr << "Interface variable of neighbor[" << *n_it <<"]: " << neighbor_V << std::endl;
#endif

	      const Signature& neighbor_sig = query_plan->getSignature(*n_it);
	      const SignatureByLocal& neighbor_loc = boost::get<Tag::Local>(neighbor_sig);


	      for (std::size_t i = 1; i < sizeof(neighbor_V)*8; ++i)
		{
		  if (neighbor_V & (1 << i))
		    {
		      SignatureByLocal::const_iterator neighbor_it = neighbor_loc.find(i);
		      std::size_t local_id_here = sig->size()+1;
		      Symbol s(neighbor_it->sym, neighbor_it->ctxId, local_id_here, neighbor_it->origId);
		      std::pair<Signature::iterator, bool> p = sig->insert(s);

		      if (p.second)
			{
			  insert_iterators.push_back(p.first);
			}
		    }
		}

	    }

#ifdef DEBUG
	  std::cerr << "Updated signature: " << *sig << std::endl;
#endif

	  builder.visitTheory(theory, sig->size());
	}
      catch (std::ios_base::failure& e)
	{
	  std::cerr << "Error: " << e.what() << std::endl;
	}
      proc.endoffile();

      // parse result

#if defined(DEBUG)
      std::cerr << "Parsing the models..." << std::endl;
#endif // DEBUG

      Parser builder(context, belief_states);

      ParserDirector<ParserGrammar> parser_director;
      parser_director.setBuilder(&builder);
      parser_director.parse(proc.getInput());      

#ifdef DEBUG      
      std::cerr << "Erasing..." << std::endl;
#endif

      SignatureIterators::const_iterator s_it = insert_iterators.begin();
      for(; s_it != insert_iterators.end(); ++ s_it)
	{
	  sig->erase(*s_it);

#ifdef DEBUG
	  std::cerr << "Restored signature: " << *sig << std::endl;
#endif
	}


      retcode = proc.close();
    }
  catch(std::exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
    }
}

} // namespace dmcs

#endif // _SAT_SOLVER_TCC

// Local Variables:
// mode: C++
// End:
