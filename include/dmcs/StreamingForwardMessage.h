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
 * @file   StreamingForwardMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  20 8:05:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_FORWARD_MESSAGE_H
#define STREAMING_FORWARD_MESSAGE_H

#include "Message.h"
#include "mcs/BeliefState.h"

namespace dmcs {

class StreamingForwardMessage : public Message
{
public:
  StreamingForwardMessage()
  { }

  virtual ~StreamingForwardMessage() 
  {}

  StreamingForwardMessage(std::size_t invoker_, std::size_t system_size)
    : invoker(invoker_), conflict(new BeliefState(system_size, BeliefSet()))
  { }

  StreamingForwardMessage(std::size_t invoker_, BeliefStatePtr conflict_)
    : invoker(invoker_), conflict(conflict_)
  { }

  std::size_t
  getInvoker() const
  {
    return invoker;
  }

  BeliefStatePtr
  getConflict() const
  {
    return conflict;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & invoker;
    ar & conflict;
  }

private:
  std::size_t invoker;     // ID of the invoking context
  BeliefStatePtr conflict; // a conflict that the receiver should obey
};

inline std::ostream&
operator<< (std::ostream& os, const StreamingForwardMessage& sfMess)
{

  os << sfMess.getInvoker() << " ["
     << sfMess.getConflict() << "] ";
  
  return os;
}

} // namespace dmcs

#endif // STREAMING_FORWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
