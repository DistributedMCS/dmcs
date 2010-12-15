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
 * @file   OptMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon May  3 10:49:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef OPT_MESSAGE_H
#define OPT_MESSAGE_H

#include "Message.h"

namespace dmcs {

class OptMessage : public Message
{
public:
  OptMessage()
  { }

  virtual ~OptMessage() 
  {}
  OptMessage(std::size_t invoker_)
    : invoker(invoker_)
  { }

  std::size_t
  getInvoker() const
  {
    return invoker;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & invoker;
  }

private:
  std::size_t invoker;
};

inline std::ostream&
operator<< (std::ostream& os, const OptMessage& optMess)
{

  os << optMess.getInvoker() << " ";
  
  return os;
}

} // namespace dmcs

#endif // OPT_MESSAGE_H

// Local Variables:
// mode: C++
// End:
