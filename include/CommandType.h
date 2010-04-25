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

namespace dmcs {

class CommandType
{
public:
  // i removed the abstract because of this error "initializer specified for non-virtual method"
  BeliefStateListPtr 
  getBeliefStates();
};



class OptCommandType : public CommandType
{
public:

  OptCommandType(BaseDMCS& d, Message& mess_)
    :dmcs(d),
     mess(mess_)
  { }

  BeliefStateListPtr
  getBeliefStates()
  {
    ///@todo get rid of copying
    // not sure if it will work
    OptMessage* optMess;
  
    OptDMCS* odmcs;

    optMess = dynamic_cast<OptMessage*>(&mess);
    odmcs = dynamic_cast<OptDMCS*>(&dmcs);
    
    
    return odmcs->getBeliefStates(*optMess);
  }

private:

  BaseDMCS& dmcs;
  Message& mess;
};



class PrimitiveCommandType : public CommandType
{
public:

  PrimitiveCommandType(BaseDMCS& d, Message& mess_)
    :dmcs(d),
     mess(mess_)
  { }

  BeliefStateListPtr 
  getBeliefStates()
  {
    ///@todo get rid of copying
    // not sure if it will work
    PrimitiveMessage* primMess;
    PrimitiveDMCS* pdmcs;

    primMess = dynamic_cast<PrimitiveMessage*>(&mess);
    pdmcs = dynamic_cast<PrimitiveDMCS*>(&dmcs);
    
    return pdmcs->getBeliefStates(*primMess);
  }

private:

  BaseDMCS& dmcs;
  Message& mess;
};


} // namespace dmcs

#endif // COMMAND_TYPE_H

// Local Variables:
// mode: C++
// End:
