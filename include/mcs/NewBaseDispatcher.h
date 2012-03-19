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
 * @file   NewBaseDispatcher.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 21:44:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_BASE_DISPATCHER_H
#define NEW_BASE_DISPATCHER_H

#include <map>
#include "network/NewConcurrentMessageDispatcher.h"

namespace dmcs {

class NewBaseDispatcher
{
public:
  NewBaseDispatcher()
  { }

  void
  registerIdOffset(std::size_t id, std::size_t offset)
  {
    std::map<std::size_t, std::size_t>::iterator it = id2offset.find(id);
    if (it != id2offset.end())
      {
	assert (offset == it->second);
      }
    else
      {
	std::pair<std::size_t, std::size_t> e(id, offset);
	id2offset.insert(e);
      }
  }

  void
  unregisterIdOffset(std::size_t id, std::size_t offset)
  {
    std::map<std::size_t, std::size_t>::iterator it = id2offset.find(id);
    assert (it != id2offset.end());
    assert (it->second == offset);
    id2offset.erase(it);
  }

  virtual void
  startup(NewConcurrentMessageDispatcherPtr md) = 0;

protected:
  std::size_t
  get_offset(std::size_t id)
  {
    std::map<std::size_t, std::size_t>::iterator it = id2offset.find(id);
    assert (it != id2offset.end());
    return it->second;
  }

protected:
  std::map<std::size_t, std::size_t> id2offset;
};

} // namespace dmcs

#endif // NEW_BASE_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
