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
 * @file   BeliefStateOffset.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Dec  25 16:34:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BELIEF_STATE_OFFSET_H
#define BELIEF_STATE_OFFSET_H

#include "mcs/NewBeliefState.h"

namespace dmcs {

class BeliefStateOffset {

public:
  static BeliefStateOffset* 
  create(std::size_t no_bs, std::size_t size_bs);

  static BeliefStateOffset*
  instance();

  std::vector<BitMagic*>&
  getMasks();

  std::vector<std::size_t>&
  getStartingOffsets();

  std::size_t
  NO_BLOCKS() const;

  std::size_t
  SIZE_BS() const;

protected:
  BeliefStateOffset(std::size_t no_bs, std::size_t size_bs);

private:
  std::size_t no_blocks;
  std::size_t size_belief_state;
  std::vector<BitMagic*> masks;
  std::vector<std::size_t> starting_offsets;

  static BeliefStateOffset* _instance;
};

} // namespace dmcs

#endif // BELIEF_STATE_OFFSET_H

// Local Variables:
// mode: C++
// End:
