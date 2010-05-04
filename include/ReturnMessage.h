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
 * @file   ReturnMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon May  3 10:52:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef RETURN_MESSAGE_H
#define RETURN_MESSAGE_H

#include "Message.h"
#include "StatsInfo.h"


namespace dmcs {

class ReturnMessage : public Message
{
public:
  ReturnMessage()
  { }

  virtual ~ReturnMessage() 
  {}

  ReturnMessage(BeliefStateListPtr belief_states_,
		PTime sending_moment_,
		StatsInfosPtr info_)
    : belief_states(belief_states_), 
      sending_moment(sending_moment_),
      info(info_)
  { }

  BeliefStateListPtr
  getBeliefStates() const
  {
    return belief_states;
  }

  StatsInfosPtr
  getStatsInfo() const
  {
    return info;
  }

  PTime
  getSendingMoment() const
  {
    return sending_moment;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & belief_states;
    ar & sending_moment;
    ar & info;
  }

private:
  BeliefStateListPtr belief_states;
  PTime sending_moment;
  StatsInfosPtr info;
};


typedef boost::shared_ptr<ReturnMessage> ReturnMessagePtr;


inline std::ostream&
operator<< (std::ostream& os, const ReturnMessage& mess)
{
  os << *mess.getBeliefStates() << std::endl
     << boost::posix_time::to_simple_string(mess.getSendingMoment()) << std::endl
     << *mess.getStatsInfo();

  return os;
}

} // namespace dmcs


#endif // RETURN_MESSAGE_H

// Local Variables:
// mode: C++
// End:
