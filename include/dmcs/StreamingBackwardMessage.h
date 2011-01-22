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
 * @file   StreamingBackwardMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  9 17:30:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_BACKWARD_MESSAGE_H
#define STREAMING_BACKWARD_MESSAGE_H

#include "Message.h"
#include "mcs/BeliefState.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace dmcs {

class StreamingBackwardMessage : public Message
{
public:
  StreamingBackwardMessage()
  { }

  // delete pointers here?
  virtual ~StreamingBackwardMessage() 
  { }

  StreamingBackwardMessage(PartialBeliefStateVecPtr& result_)
    : result(result_)
  { }

  PartialBeliefStateVecPtr
  getBeliefStates() const
  {
    return result;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & result;
  }

private:
  PartialBeliefStateVecPtr result;
};

inline std::ostream&
operator<< (std::ostream& os, const StreamingBackwardMessage& sbMess)
{
  const PartialBeliefStateVecPtr& result = sbMess.getBeliefStates();
  os << "{ ";
  for (PartialBeliefStateVec::const_iterator it = result->begin();
       it != result->end(); ++it)
    {
      if (*it)
	{
	  os << **it << std::endl;
	}
      else
	{
	  os << "NULL" << std::endl;
	}
    }
  os << " }";

  return os;
}

typedef boost::shared_ptr<StreamingBackwardMessage> StreamingBackwardMessagePtr;

} // namespace dmcs

#endif // STREAMING_FORWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
