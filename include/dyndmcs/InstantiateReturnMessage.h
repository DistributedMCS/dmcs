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
 * @file   InstantiateReturnMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  27 12:50:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATE_RETURN_MESSAGE_H
#define INSTANTIATE_RETURN_MESSAGE_H

#include <boost/serialization/shared_ptr.hpp>

#include "Message.h"
#include "StatsInfo.h"

namespace dmcs {

class InstantiateReturnMessage : public Message
{
public:
  InstantiateReturnMessage()
  { }

  virtual ~InstantiateReturnMessage() 
  { }

  InstantiateReturnMessage(bool status_, HistoryPtr history_)
    : status(status_), history(history_)
  { }

  const bool
  getStatus() const
  {
    return status;
  }

  const HistoryPtr
  getHistory() const
  {
    return history;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & status;
    ar & history;
  }

private:
  bool status;
  HistoryPtr history; 
};


typedef boost::shared_ptr<InstantiateReturnMessage> InstantiateReturnMessagePtr;


inline std::ostream&
operator<< (std::ostream& os, const InstantiateReturnMessage& mess)
{
  os << "(" << mess.getStatus() << ") [" << *mess.getHistory() << "]" << std::endl;
  
  return os;
}

} // namespace dmcs


#endif // INSTANTIATE_RETURN_MESSAGE_H

// Local Variables:
// mode: C++
// End:
