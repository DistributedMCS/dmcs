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
 * @file   BaseType.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Sep  23 14:04:23 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_TYPE_H
#define BASE_TYPE_H

#include "mcs/Pool.h"

namespace dmcs {

struct BaseType
{
  BaseType() 
  { }
  
  void *operator new (std::size_t num, Pool *pool)
  {
    return pool->malloc();
  }
};
  
} // namespace dmcs

#endif // BASE_TYPE_H

// Local Variables:
// mode: C++
// End:
