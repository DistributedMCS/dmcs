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
 * @file   Instantitator.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan 24 2012 15:56
 *
 * @brief 
 *
 *
 */

#include "dmcs/Instantiator.h"

namespace dmcs {

Instantiator::Instantiator(const EngineWPtr& e,
			   const std::string& kbn)
  : engine(e), kbspec(kbn)
{
  inp.addFileInput(kbspec);
}


void
Instantiator::removeEvaluator(EvaluatorWPtr eval)
{
  if (eval.use_count() == 1)
    {
      EvaluatorPtr eval_p = eval.lock();
      evaluators.remove(eval_p);
    }
}


std::istream&
Instantiator::getKB()
{
  return inp.getAsStream();
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
