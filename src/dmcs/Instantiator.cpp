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
{ }


Instantiator::~Instantiator()
{ }


boost::thread*&
Instantiator::getThread(EvaluatorPtr eval)
{
  EvaluatorList::iterator eval_it = std::find(evaluators.begin(), evaluators.end(), eval);
  assert (eval_it != evaluators.end());

  std::size_t dist = std::distance(evaluators.begin(), eval_it);

  BoostThreadList::iterator thread_it = eval_threads.begin();
  std::advance(thread_it, dist);  

  return *thread_it;
}


void
Instantiator::stopThread(EvaluatorPtr eval)
{
  boost::thread*& t = getThread(eval);
  t->join();
}


void
Instantiator::removeEvaluator(EvaluatorWPtr eval)
{
  // We need at least 1 EvaluatorPtr outside to get the corresponding weak pointer
  // and one unit for the counter comes from the EvaluatorPtr in the list
  if (eval.use_count() == 2)
    {
      EvaluatorPtr eval_p = eval.lock();
      evaluators.remove(eval_p);
    }
}


std::size_t
Instantiator::getNoEvaluators()
{
  return evaluators.size();
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
