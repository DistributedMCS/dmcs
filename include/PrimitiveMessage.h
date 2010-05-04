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
 * @file   PrimitiveMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon May  3 10:47:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef PRIMITIVE_MESSAGE_H
#define PRIMITIVE_MESSAGE_H

#include "Message.h"

namespace dmcs {

typedef std::set<int> History;

class PrimitiveMessage : public Message
{
public:
  // do not use default ctor in your code, this is only needed for serialization
  PrimitiveMessage()
  { }

  virtual ~PrimitiveMessage() 
  {}

  PrimitiveMessage(BeliefStatePtr& v_)
    : v(v_)
  { } 

  PrimitiveMessage(BeliefStatePtr& v_, const History& hist_)
    : v(v_), history(hist_)
  { }

  //  std::size_t
  //getSystemSize() const
  //{
  //  return v.belief_state_ptr->belief_state.size();
  //}

  const BeliefStatePtr&
  getV() const
  {
    return v;
  }

  const History&
  getHistory() const
  {
    return history;
  }

  void
  insertHistory(std::size_t k)
  {
    history.insert(k);
  }

  void
  removeHistory(std::size_t k)
  {
    history.erase(k);
  }  

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    //assert(getSystemSize() > 0); // before we can serialize it, the system size must be positive
    ar & v;
    ar & history;
  }

private:
  BeliefStatePtr v;
  History history;
};



inline std::ostream&
operator<< (std::ostream& os, const PrimitiveMessage& primMess)
{

  os << "{ " << *primMess.getV() << "} ";
  for (History::const_iterator it = primMess.getHistory().begin(); it != primMess.getHistory().end(); ++it)
    {
      os << *it << " ";
    }
  
  return os;
}


} // namespace dmcs

#endif

// Local Variables:
// mode: C++
// End:
