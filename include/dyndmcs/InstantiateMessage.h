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
 * @file   InstantiateMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Sep  23 14:55:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATE_MESSAGE_H
#define INSTANTIATE_MESSAGE_H

#include <iterator>
#include <list>

#include "dyndmcs/ContextSubstitution.h"
#include "dmcs/Message.h"
#include "mcs/BeliefState.h"

namespace dmcs {

class InstantiateMessage : public Message
{
public:
  InstantiateMessage()
    : history(new History), interface_vars(new BeliefState)
  { }

  virtual ~InstantiateMessage() 
  { }

  InstantiateMessage(HistoryPtr history_, BeliefStatePtr interface_vars_)
    : history(history_), interface_vars(interface_vars_)
  { }

  ///@todo: merge PrimitiveMessage and InstantiateMessage somehow,
  ///sice they share history.
  HistoryPtr
  getHistory() const
  {
    return history;
  }

  BeliefStatePtr
  getInterfaceVars() const
  {
    return interface_vars;
  }

  void
  setInterface(std::size_t ctx_id, std::size_t atom_position)
  {
    // from ctx_id, find the position of the BeliefSet in interface_vars
    History::iterator hist_it = std::find(history->begin(), history->end(), ctx_id);

    assert( hist_it != history->end() );

    std::size_t d = std::distance(history->begin(), hist_it);
    BeliefState::const_iterator iv_it = interface_vars->begin();
    std::advance(iv_it, d);

    // then set the bit at atom_position
    BeliefSet bs = *iv_it;
    bs.set(atom_position);
  }

  void
  insertHistory(std::size_t k)
  {
    history->push_back(k);
  }

  void
  removeHistory()
  {
    history->pop_back();
  }   

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & history;
    ar & interface_vars;
  }

protected:
  HistoryPtr history;
  BeliefStatePtr interface_vars;
};


inline std::ostream&
operator<< (std::ostream& os, const InstantiateMessage& mess)
{
  os << "[" << *mess.getHistory() << "]" << std::endl
     << "[" << *mess.getInterfaceVars() << "]" << std::endl;
  return os;
}


typedef boost::shared_ptr<InstantiateMessage> InstantiateMessagePtr;

} // namespace dmcs

#endif // INSTANTIATE_MESSAGE_H

// Local Variables:
// mode: C++
// End:
