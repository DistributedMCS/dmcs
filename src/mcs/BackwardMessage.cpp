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
 * @file   BackwardMessage.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan 1 15:52:59 2012
 * 
 * @brief  
 * 
 * 
 */
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "mcs/BackwardMessage.h"

namespace dmcs {

BackwardMessage::BackwardMessage()
{ }



BackwardMessage::~BackwardMessage()
{ }



BackwardMessage::BackwardMessage(const BackwardMessage& bMess)
{
  rbsl = bMess.rbsl;
}



BackwardMessage::BackwardMessage(const ReturnedBeliefStateListPtr& r)
  : rbsl(r)
{ }



std::ostream&
BackwardMessage::print(std::ostream& os) const
{
  return os << printlist(*rbsl, "", "\n", "");
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
