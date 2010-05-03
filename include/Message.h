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
 * @file   Message.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Nov  18 11:53:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include "BeliefState.h"
#include "StatsInfo.h"
#include "Context.h"

#include <set>
#include <boost/serialization/set.hpp>


namespace dmcs {

class Message
{
public:
  virtual ~Message() 
  {}
  // templates cant be virtual
  template<class Archive>
  void serialize(Archive & /* ar */, const unsigned int /* version */)
  {}
};

} // namespace dmcs

#endif // MESSAGE_H

// Local Variables:
// mode: C++
// End:
