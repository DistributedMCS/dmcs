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
 * @file   DLVEngine.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan 25 2012 12:01
 *
 * @brief 
 *
 *
 */

#include "dmcs/DLVEngine.h"
#include "dmcs/DLVInstantiator.h"

namespace dmcs {

DLVEngine::DLVEngine()
{ }


DLVEngine::~DLVEngine()
{ }


DLVEnginePtr
DLVEngine::create()
{
  DLVEnginePtr dlv_eng(new DLVEngine);
  return dlv_eng;
}


InstantiatorPtr
DLVEngine::createInstantiator(const EngineWPtr& eng,
			      const std::string& kbspec)
{
  EnginePtr eng_p = eng.lock();
  Engine* eng_s = eng_p.get();
  assert (this == eng_s);

  InstantiatorPtr inst = DLVInstantiator::create(eng, kbspec);
  instantiators.push_back(inst);

  return inst;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
