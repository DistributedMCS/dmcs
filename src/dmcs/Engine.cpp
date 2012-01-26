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
 * @file   Engine.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Nov 28 2011 16:50
 *
 * @brief 
 *
 *
 */

#include "dmcs/Engine.h"

namespace dmcs {

Engine::Engine()
{ }


Engine::~Engine()
{ }


void
Engine::removeInstantiator(InstantiatorWPtr inst)
{
  // See comment in Instantiator::removeEvaluator(EvaluatorWPtr eval)
  if (inst.use_count() == 2)
    {
      InstantiatorPtr inst_p = inst.lock();
      instantiators.remove(inst_p);
    }
}


std::size_t
Engine::getNoInstantiators()
{
  return instantiators.size();
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
