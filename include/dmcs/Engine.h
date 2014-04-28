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
 * @file   Engine.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Nov 28 2011 16:50
 *
 * @brief 
 *
 *
 */


#ifndef ENGINE_H
#define ENGINE_H

#include <list>

#include "dmcs/AbstractContext.h"
#include "dmcs/Instantiator.h"

namespace dmcs {

class Engine
{
public:
  static EnginePtr
  create();

  virtual 
  ~Engine();

  // kbspec may simply contain a filename or something engine-specific
  virtual InstantiatorPtr
  createInstantiator(const EngineWPtr& eng,
		     const std::string& kbspec) = 0;

  // call this with a weak pointer to unregister the instantiator
  //
  // in debug mode, removeInstantiator will check whether the
  // usage count of inst is 1, so it will ensure that inst is really destructed
  // (passing a non-weak pointer as argument would be misleading,
  // as passing a non-weak pointer will also occupy at least one usage count)
  virtual void
  removeInstantiator(InstantiatorWPtr inst);

  std::size_t
  getNoInstantiators();

protected:
  // objects must be created via create()
  Engine();

  std::list<InstantiatorPtr> instantiators;
};


} // namespace dmcs


#endif // ENGINE_H

// Local Variables:
// mode: C++
// End:
