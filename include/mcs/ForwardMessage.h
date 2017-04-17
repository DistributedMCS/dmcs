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
 * @file   ForwardMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  23 10:22:17 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef FORWARD_MESSAGE_H
#define FORWARD_MESSAGE_H

#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>

#include "mcs/Printhelpers.h"
#include "mcs/NewMessage.h"

namespace dmcs {

typedef std::set<std::size_t> NewHistory;

struct ForwardMessage : 
    public NewMessage,
    private ostream_printable<ForwardMessage>
{
  ForwardMessage();

  virtual
  ~ForwardMessage(); 

  ForwardMessage(const ForwardMessage& fMess);

  ForwardMessage(std::size_t qid,
		 const NewHistory& history,
		 std::size_t k_one = 0,
		 std::size_t k_two = 0);

  std::size_t
  getPackSize() const;

  void
  setPackRequest(const std::size_t k_one, 
		 const std::size_t k_two);

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & qid;
    ar & k1;
    ar & k2;
    ar & history;
  }

  std::ostream&
  print(std::ostream& os) const;

  std::size_t qid;
  std::size_t k1;         // The invoker wants models from k1 to k2
  std::size_t k2;
  NewHistory history;
};

typedef boost::shared_ptr<ForwardMessage> ForwardMessagePtr;

} // namespace dmcs



#endif // FORWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
