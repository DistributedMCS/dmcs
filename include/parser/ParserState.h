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
 * @file   ParserState.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  12 12:40:20 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include "mcs/BeliefStateOffset.h"
#include "parser/DLVResultParser.h"

namespace dmcs {

struct ParserState
{
  std::size_t ctx_id;
  BeliefTablePtr btab;
  NewBeliefState* current;
  DLVResultParser::BeliefStateAdder adder;

  ParserState(std::size_t cid,
	      BeliefTablePtr b,
	      DLVResultParser::BeliefStateAdder a)
    : ctx_id(cid),
      btab(b),
      adder(a)
  {
    reset_current();
  }

  void
  reset_current()
  {
    current = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
				 BeliefStateOffset::instance()->SIZE_BS());

    const std::vector<std::size_t>& starting_offsets = BeliefStateOffset::instance()->getStartingOffsets();
    current->setEpsilon(ctx_id, starting_offsets);

    std::size_t start_bit = starting_offsets[ctx_id] + 1;
    std::size_t end_bit;
    if (ctx_id == starting_offsets.size() - 1)
      end_bit = current->status_bit.size();
    else
      end_bit = starting_offsets[ctx_id+1];

    for (std::size_t i = start_bit; i < end_bit; ++i)
      current->set(i, NewBeliefState::DMCS_FALSE);
  }
};

} // namespace dmcs

#endif // PARSER_STATE_H

// Local Variables:
// mode: C++
// End:
