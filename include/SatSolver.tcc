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



template <typename Builder, typename Parser, typename ParserGrammar>
void
SatSolver<Builder, Parser, ParserGrammar>::solve(const Context& context,
						 BeliefStatesPtr& belief_states,
						 const TheoryPtr& theory)
{
  int retcode = -1;
  
  try
    {
      proc.spawn();
      try 
	{
	  Builder builder(proc.getOutput());
	  builder.visitTheory(theory, *context.getSignature());
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
