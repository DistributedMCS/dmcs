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
 * @file   ConfigMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed May  26 15:19:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef CONFIG_MESSAGE_H
#define CONFIG_MESSAGE_H

#include "Message.h"

namespace dmcs {

class ConfigMessage : public Message
{
public:
  ConfigMessage()
  { }

  virtual ~ConfigMessage() 
  { }

  ConfigMessage(std::size_t root_ctx_, ContextSubstitutionPtr ctx_substitution_, bool is_last_)
    : root_ctx(root_ctx_), ctx_substitution(ctx_substitution_), is_last(is_last_)
  { }

  std::size_t
  getRootContext() const
  {
    return root_ctx;
  }

  const ContextSubstitutionPtr
  getCtxSubstitution() const
  {
    return ctx_substitution;
  }

  bool
  isLastMess() const
  {
    return is_last;
  }

public:
  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & root_ctx;
    ar & ctx_substitution;
    ar & is_last;
  }

private:
  std::size_t root_ctx;
  ContextSubstitutionPtr ctx_substitution;
  bool is_last;
};


inline std::ostream&
operator<< (std::ostream& os, const ConfigMessage& config_mess)
{
  os << config_mess.getRootContext() << "[" << *config_mess.getCtxSubstitution() << "](" << config_mess.isLastMess() << ")" << std::endl;
  return os;
}


typedef boost::shared_ptr<ConfigMessage> ConfigMessagePtr;

} // namespace dmcs

#endif // CONFIG_MESSAGE_H
