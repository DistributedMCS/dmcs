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
 * @file   DynamicCommandType.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri May  28 16:18:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DYNAMIC_COMMAND_TYPE
#define DYNAMIC_COMMAND_TYPE

#include "CommandType.h"
#include "dyndmcs/DynamicConfiguration.h"

namespace dmcs {

class DynamicCommandType : public CommandType<ConfigMessage, ContextSubstitutionList>
{
public:
  
  typedef DynamicConfiguration::dynmcs_value_type value_type;

  DynamicCommandType(DynamicConfigurationPtr dconf_)
    : dconf(dconf_)
  { }

  return_type
  execute(ConfigMessage& mess)
  {

#ifdef DEBUG
    std::cerr << "going to call dconf with mess = " << mess << std::endl;
#endif

    std::cerr << "dconf->getSignal() = " << dconf->getSignal() << std::endl;
    //std::cerr << dconf->getBridgeRules() << std::endl;

    return dconf->lconfig(mess);
  }

  bool
  continues(ConfigMessage& mess)
  {
    return mess.isLastMess();
  }

  DynamicConfigurationPtr getDconf()
  {
    return dconf;
  }

private:
  DynamicConfigurationPtr dconf;
};

typedef boost::shared_ptr<DynamicCommandType> DynamicCommandTypePtr;

} // namespace dmcs

#endif // DYNAMIC_COMMAND_TYPE

// Local Variables:
// mode: C++
// End:
