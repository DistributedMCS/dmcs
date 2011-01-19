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

#include "dmcs/Log.h"

#include "parser/BaseBuilder.h"
#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include <exception>


namespace dmcs {


template <typename Builder, typename Parser, typename ParserGrammar>
SatSolver<Builder, Parser, ParserGrammar>::SatSolver(Process<Parser>* p, Parser* mb)
  : proc(p),
    model_builder(mb)
{ }


typedef std::list<Signature::const_iterator> SignatureIterators;

template <typename Builder, typename Parser, typename ParserGrammar>
int
SatSolver<Builder, Parser, ParserGrammar>::solve(const TheoryPtr& theory, std::size_t sig_size)
{
  int retcode = -1;
  
  try
    {
      assert(proc != 0);

      //
      // send the theory to the SAT solver 
      //
     
      proc->spawn();
      
      Builder builder(proc->getOutput());
      builder.visitTheory(theory, sig_size);

#ifdef DEBUG
      Builder evil_builder(std::cerr);
      evil_builder.visitTheory(theory, sig_size);
#endif
      
      proc->endoffile();
      
      //
      // parse result and set belief states
      //

      DMCS_LOG_TRACE("Parsing the models...");
      
      //Parser model_builder(context_signature, belief_states);
      
      ParserDirector<ParserGrammar> parser_director;
      parser_director.setBuilder(model_builder);
      parser_director.parse(proc->getInput());      

      //
      // close the connection to the SAT solver
      //

      retcode = proc->close();
    }
  catch (std::ios_base::failure& e)
    {
      DMCS_LOG_ERROR("Parsing error in SatSolver: " << e.what());
      throw e;
    }
  catch(std::exception& e)
    {
      DMCS_LOG_ERROR("Parsing error in SatSolver: " << e.what());
      throw e;
    }
  
  return retcode;
}
  
} // namespace dmcs

#endif // _SAT_SOLVER_TCC

// Local Variables:
// mode: C++
// End:
