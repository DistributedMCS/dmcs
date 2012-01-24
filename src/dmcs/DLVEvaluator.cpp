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
 * @file   DLVEvaluator.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  19 15:10:24 2012
 * 
 * @brief  
 * 
 * 
 */

#include "DLVEvaluator.h"

namespace dmcs {

DLVEvaluator::Options::Options()
  : Evaluator::GenericOptions(),
    arguments()
{
  arguments.push_back("-silent");
}



DLVEvaluator::Options::~Options()
{ }


DLVEvaluator::DLVEvaluator(const InstantiatorWPtr& inst,
			   const NewConcurrentMessageDispatcherPtr d)
  : Evaluator(inst, d)
{ }



void
DLVEvaluator::solve(NewBeliefState* heads)
{
  // setupProcess
  proc.setPath(DLVPATH);
  if (options.includeFacts)
    {
      proc.addOption("-facts");
    }
  else
    {
      proc.addOption("-nofacts");
    }

  BOOST_FOREACH(const std::string& arg, options.arguments)
    {
      proc.addOption(arg);
    }

  // request stdin as last parameter
  proc.addOption("--");
  
  // fork dlv process
  proc.spawn();

  std::ostream& programStream = proc.getOutput();

  // copy stream
  std::istream& input_stream = instantiator->getKB();
  programStream << input_stream.rdbuf();
  programStream.flush();

  proc.endoffile();
  
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
