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
 * @file   InstantiatorCommandType.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri May  28 16:18:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATOR_COMMAND_TYPE
#define INSTANTIATOR_COMMAND_TYPE

#include "CommandType.h"
#include "Instantiator.h"
#include "ContextSubstitution.h"

namespace dmcs {

class InstantiatorCommandType : public CommandType<InstantiateForwardMessage, InstantiateBackwardMessage>
{
public:
  
  InstantiatorCommandType(InstantiatorPtr& inst_)
    : inst(inst_)
  { }

  return_type
  execute(InstantiateForwardMessage& mess)
  {
    return inst->instantiate(mess);
  }

  bool
  continues(InstantiateForwardMessage& /* mess */)
  {
    return false;
  }

  void
  createNeighborInputThreads(ThreadVecPtr /* tv */)
  { }

  void
  createDMCSThread(boost::thread* /* t */, const InstantiateForwardMessage&)
  { }

  void
  createLocalSolveThread(boost::thread* /* t */)
  { }

private:
  InstantiatorPtr inst;
};

typedef boost::shared_ptr<InstantiatorCommandType> InstantiatorCommandTypePtr;

} // namespace dmcs

#endif // INSTANTIATOR_COMMAND_TYPE

// Local Variables:
// mode: C++
// End:
