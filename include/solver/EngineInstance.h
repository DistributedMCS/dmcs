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
 * @file   EngineInstance.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  19 12:28:30 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ENGINE_INSTANCE_H
#define ENGINE_INSTANCE_H

#include "solver/Solver.h"

namespace dmcs {

class EngineInstance
{
public:
  EngineInstance(const std::string& kbn)
    : kbname(kbn)
  { 
    init();
  }
 
  virtual Solver*
  createSolver(ConcurrentMessageQueue input_queue, ConcurrentMessageQueue output_queue) = 0;
  
private:
  virtual void init() = 0;

private:
  std::string kbname;
};



class DLVEngineInstance : public EngineInstance
{
public:
  DLVEngineInstance(const std::string& kbname)
    : EngineInstance(kbname);
  { }

  Solver*
  createSolver(ConcurrentMessageQueue input_queue, ConcurrentMessageQueue output_queue)
  { }

private:
  void
  init();

private:
  std::vector<std::string> kb;
};


} // namespace dmcs

#endif // ENGINE_INSTANCE_H

// Local Variables:
// mode: C++
// End:
