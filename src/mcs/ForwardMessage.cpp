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
 * @file   ForwardMessage.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec 23 10:37:59 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/ForwardMessage.h"

namespace dmcs {

ForwardMessage::ForwardMessage()
  : qid(0), k1(1), k2(1)
{ }


ForwardMessage::~ForwardMessage()
{ }


ForwardMessage::ForwardMessage(const ForwardMessage& fMess)
{
  qid = fMess.qid;
  k1 = fMess.k1;
  k2 = fMess.k2;
  history = fMess.history;
}


ForwardMessage::ForwardMessage(std::size_t q,
			       const NewHistory& history,
			       std::size_t k_one,
			       std::size_t k_two)
  : qid(q), 
    k1(k_one),
    k2(k_two),
    history(history)
{ }


std::size_t
ForwardMessage::getPackSize() const
{
  if (k2 == 0)
    {
      return 0;
    }
  
  return k2 - k1 + 1;
}


void
ForwardMessage::setPackRequest(const std::size_t k_one, 
			       const std::size_t k_two)
{
  assert (k_one <= k_two);
  
  k1 = k_one;
  k2 = k_two;
}


std::ostream&
ForwardMessage::print(std::ostream& os) const
{
  os << "{" << qid << "}" << "[" << k1 << "," << k2 << "] {";
  std::copy(history.begin(), history.end(), std::ostream_iterator<std::size_t>(os, " "));
  os << "}";
  return os;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
