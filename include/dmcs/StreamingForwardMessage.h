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

#include "dmcs/Log.h"
#include "dmcs/Message.h"
#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

namespace dmcs {

class StreamingForwardMessage : public Message
{
public:
  // public default ctor, everything 0 for now
  StreamingForwardMessage()
    :
    //#ifdef DEBUG
    //    path(1, 0),
    //#else
    path(0),
    //#endif
    invoker(0),
    session_id(0),
    k1(1),
    k2(1)
  { }

  virtual
  ~StreamingForwardMessage() 
  { }

  StreamingForwardMessage(const StreamingForwardMessage& sfMess)
  {
    path = sfMess.path;
    invoker = sfMess.invoker;
    session_id = sfMess.session_id;
    k1 = sfMess.k1;
    k2 = sfMess.k2;
  }

  StreamingForwardMessage(std::size_t p,
			  std::size_t i,
			  std::size_t sid,
			  std::size_t k_one,
			  std::size_t k_two)
    : path(p),
      invoker(i),
      session_id(sid), 
      k1(k_one),
      k2(k_two)
  { }

  std::size_t
  getSessionId() const
  {
    return session_id;
  }

  std::size_t
  getInvoker() const
  {
    return invoker;
  }

  std::size_t
  getPath() const
  {
    return path;
  }

  std::size_t
  getPackSize() const
  {
    return k2 - k1 + 1;
  }

  std::size_t
  getK1() const
  {
    return k1;
  }

  void
  setK1(std::size_t k_one)
  {
    k1 = k_one;
  }

  std::size_t
  getK2() const
  {
    return k2;
  }

  void
  setPackRequest(std::size_t k_one, std::size_t k_two)
  {
    assert (k_one <= k_two);

    k1 = k_one;
    k2 = k_two;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & path;
    ar & invoker;
    ar & session_id;
    ar & k1;
    ar & k2;
  }

private:
  std::size_t path;
  std::size_t invoker;
  std::size_t session_id; // For filtering old models
  std::size_t k1;         // The invoker wants models from k1 to k2
  std::size_t k2;

};



typedef boost::shared_ptr<StreamingForwardMessage> StreamingForwardMessagePtr;



inline std::ostream&
operator<< (std::ostream& os, const StreamingForwardMessage& sfMess)
{

  os << "path = {" << sfMess.getPath() << "}, "
     << "invoker = " << sfMess.getInvoker() << ", "
     << "session_id = " << sfMess.getSessionId() << ", " 
     << "k1 = " << sfMess.getK1() << ", " 
     << "k2 = " << sfMess.getK2();
  
  return os;
}



} // namespace dmcs



#endif // STREAMING_FORWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
