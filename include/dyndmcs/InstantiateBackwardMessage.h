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
 * @file   InstantiateBackwardMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Oct  05 10:02:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATE_BACKWARD_MESSAGE_H
#define INSTANTIATE_BACKWARD_MESSAGE_H

#include <boost/serialization/shared_ptr.hpp>

#include "dyndmcs/InstantiateMessage.h"
#include "dmcs/StatsInfo.h"

namespace dmcs {

class InstantiateBackwardMessage : public InstantiateMessage
{
public:
  InstantiateBackwardMessage()
  { }

  virtual ~InstantiateBackwardMessage() 
  { }

  InstantiateBackwardMessage(bool status_, HistoryPtr history_, BeliefStatePtr interface_vars_)
    : InstantiateMessage(history_, interface_vars_), status(status_)
  { }

  const bool
  getStatus() const
  {
    return status;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & history;
    ar & status;
  }

private:
  bool status;
};


typedef boost::shared_ptr<InstantiateBackwardMessage> InstantiateBackwardMessagePtr;


inline std::ostream&
operator<< (std::ostream& os, const InstantiateBackwardMessage& mess)
{
  os << "(" << mess.getStatus() << ")" << std::endl 
     << "[" << *mess.getHistory() << "]" << std::endl
     << "[" << *mess.getInterfaceVars() << "]" << std::endl;
  return os;
}

} // namespace dmcs


#endif // INSTANTIATE_RETURN_MESSAGE_H

// Local Variables:
// mode: C++
// End:
