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
 * @file   CommandType.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Wed Jan 25 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef COMMAND_TYPE_H
#define COMMAND_TYPE_H

#include "BeliefState.h"
#include "OptDMCS.h"
#include "PrimitiveDMCS.h"


namespace dmcs {

template<typename MessageType, typename RetVal>
class CommandType
{
public:
  typedef MessageType input_type;
  typedef RetVal value_type;
  typedef boost::shared_ptr<value_type> return_type;

  return_type
  execute(input_type& mess);

  virtual bool
  continues(input_type& mess) = 0;
};



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



class PrimitiveCommandType 
  : public CommandType<PrimitiveMessage, PrimitiveDMCS::dmcs_value_type>
{
public:
  typedef PrimitiveDMCS::dmcs_value_type value_type;

  PrimitiveCommandType(PrimitiveDMCSPtr pdmcs_)
    : pdmcs(pdmcs_)
  { }

  return_type
  execute(PrimitiveMessage& mess)
  {
    return pdmcs->getBeliefStates(mess);
  }

  bool
  continues(PrimitiveMessage& /* mess */)
  {
    return false;
  }

private:
  PrimitiveDMCSPtr pdmcs;
};


} // namespace dmcs

#endif // COMMAND_TYPE_H

// Local Variables:
// mode: C++
// End:
