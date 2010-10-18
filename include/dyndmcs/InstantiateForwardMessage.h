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
 * @file   InstantiateForwardMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Oct  05 09:52:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef INSTANTIATE_FORWARD_MESSAGE_H
#define INSTANTIATE_FORWARD_MESSAGE_H

#include "InstantiateMessage.h"


namespace dmcs {

class InstantiateForwardMessage : public InstantiateMessage
{
public:
  InstantiateForwardMessage()
  { }

  virtual ~InstantiateForwardMessage() 
  { }

  InstantiateForwardMessage(ContextSubstitutionPtr ctx_substitution_)
    : InstantiateMessage(), ctx_substitution(ctx_substitution_)
  { }

  InstantiateForwardMessage(ContextSubstitutionPtr ctx_substitution_, HistoryPtr history_,
			    BeliefStatePtr interface_vars_)
    : InstantiateMessage(history_, interface_vars_), ctx_substitution(ctx_substitution_)
  { }

  const ContextSubstitutionPtr
  getCtxSubstitution() const
  {
    return ctx_substitution;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & history;
    ar & ctx_substitution;
    //ar & interface_vars;
  }

private:
  ContextSubstitutionPtr ctx_substitution;
};


inline std::ostream&
operator<< (std::ostream& os, const InstantiateForwardMessage& mess)
{
  os << "[" << *mess.getCtxSubstitution() << "] " << std::endl 
     << "[" << *mess.getHistory() << "]" << std::endl
     << "[" << *mess.getInterfaceVars() << "]" << std::endl;
  return os;
}


typedef boost::shared_ptr<InstantiateForwardMessage> InstantiateForwardMessagePtr;

} // namespace dmcs

#endif // INSTANTIATE_FORWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
