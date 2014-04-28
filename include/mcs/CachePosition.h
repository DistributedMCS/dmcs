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
 * @file   CachePosition.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Dec  4 10:35:26 2012
 * 
 * @brief  
 */

#ifndef ___CACHE_H___
#define ___CACHE_H___

#include <vector>

namespace dmcs {

class CachePosition {
public:
  void
  update_cache(std::size_t index,
	       std::size_t k);

  std::size_t
  find_position(const std::size_t k);

private:
  std::vector<std::size_t> mark;
};

} // namespace dmcs

#endif // ___CACHE_H___

// Local Variables:
// mode: C++
// End:
