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
#ifdef DEBUG
    path(1, 0),
#else
      path(0),
#endif
      invoker(0),
      session_id(0),
      pack_size(0)
  { }

  virtual
  ~StreamingForwardMessage() 
  { }

  StreamingForwardMessage(const StreamingForwardMessage& sfMess)
  {
    path = sfMess.path;
    invoker = sfMess.invoker;
    session_id = sfMess.session_id;
    pack_size = sfMess.session_id;
  }

  StreamingForwardMessage(
#ifdef DEBUG
			  History p,
#else
			  std::size_t p,
#endif
			  std::size_t i,
			  std::size_t sid,
			  std::size_t ps)
    : path(p),
      invoker(i),
      session_id(sid), 
      pack_size(ps)
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

#ifdef DEBUG
  History
#else
  std::size_t
#endif
  getPath() const
  {
    return path;
  }

  std::size_t
  getPackSize() const
  {
    return pack_size;
  }

  void
  setPackSize(std::size_t k)
  {
    pack_size = k;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & path;
    ar & invoker;
    ar & session_id;
    ar & pack_size;
  }

private:

#ifdef DEBUG
  History path;           // For debugging purpose
#else
  std::size_t path;       // Hashed value for the calling path, used
			  // for dispatching independent queries from
			  // the same parent
#endif

  std::size_t invoker;
  std::size_t session_id; // For filtering old models
  std::size_t pack_size;  // The number of models in a package that
			  // the invoker expects
};



typedef boost::shared_ptr<StreamingForwardMessage> StreamingForwardMessagePtr;



inline std::ostream&
operator<< (std::ostream& os, const StreamingForwardMessage& sfMess)
{

  os << "path = {" << sfMess.getPath() << "}, "
     << "invoker = " << sfMess.getInvoker() << ", "
     << "session_id = " << sfMess.getSessionId() << ", " 
     << "pack_size = " << sfMess.getPackSize();
  
  return os;
}



} // namespace dmcs



#endif // STREAMING_FORWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
