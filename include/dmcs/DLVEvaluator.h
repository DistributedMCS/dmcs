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
 * @file   DLVEvaluator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan  18 23:32:24 2012
 * 
 * @brief  
 * 
 * 
 */


#ifndef DLV_EVALUATOR_H
#define DLV_EVALUATOR_H

#include "dmcs/Evaluator.h"
#include "process/DLVProcess.h"

namespace dmcs {

class DLVEvaluator : public Evaluator
{
public:
  struct Options : public Evaluator::GenericOptions
  {
    Options();
    virtual ~Options();

    std::vector<std::string> arguments;
  };

  DLVEvaluator(const InstantiatorWPtr& inst,
	       const NewConcurrentMessageDispatcherPtr d);

  void
  solve(NewBeliefState* heads);

private:
  DLVProcess proc;
};

} // namespace dmcs

#endif // DLV_EVALUATOR_H

// Local Variables:
// mode: C++
// End:
