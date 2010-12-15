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
 * @file   OptCommandType.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri May  28 16:18:01 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef OPT_COMMAND_TYPE
#define OPT_COMMAND_TYPE

#include "CommandType.h"
#include "OptDMCS.h"

namespace dmcs {

class OptCommandType : public CommandType<OptMessage, OptDMCS::dmcs_value_type>
{
public:
  typedef OptMessage input_type;
  typedef OptDMCS::dmcs_value_type value_type;

  OptCommandType(OptDMCSPtr odmcs_)
    : odmcs(odmcs_)
  { }

  return_type
  execute(const OptMessage& mess)
  {
    return odmcs->getBeliefStates(mess);
  }

  bool
  continues(OptMessage& /* mess */)
  {
    return false;
  }

private:
  OptDMCSPtr odmcs;
};


typedef boost::shared_ptr<OptCommandType> OptCommandTypePtr;

} // namespace dmcs

#endif // OPT_COMMAND_TYPE

// Local Variables:
// mode: C++
// End:
