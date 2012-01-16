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
 * @file   DLVResultGrammar.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  12 12:25:14 2012
 * 
 * @brief  
 * 
 * 
 */

#include "parser/DLVResultGrammar.h"

namespace dmcs {

handle_literal::handle_literal(ParserState& state)
  : state(state)
{ }



void 
handle_literal::operator()(boost::fusion::vector2<boost::optional<char>,
					       std::string>& attr, 
			     qi::unused_type, qi::unused_type) const
{
  bool strong_neg = boost::fusion::at_c<0>(attr);
  const std::string& belief_text = boost::fusion::at_c<1>(attr);
  Belief belief(state.ctx_id, belief_text);
  ID belief_id = state.btab->storeAndGetID(belief);
  
  if (strong_neg)
    {
      state.current->set(state.ctx_id, 
			 belief_id.address,
			 BeliefStateOffset::instance()->getStartingOffsets(),
			 NewBeliefState::DMCS_FALSE);
    }
  else
    {
      state.current->set(state.ctx_id, 
			 belief_id.address,
			 BeliefStateOffset::instance()->getStartingOffsets());
    }
}



handle_finished_answerset::handle_finished_answerset(ParserState& state)
  : state(state) 
{ }



void 
handle_finished_answerset::operator()(qi::unused_type, qi::unused_type, qi::unused_type) const
{
  // add current answer set as full answer set
  //DBGLOG(DBG,"handling parsed answer set " << *state.current);

  state.adder(state.current);
  // create empty answer set for subsequent parsing
  state.current = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
				     BeliefStateOffset::instance()->SIZE_BS());
}

} // namespacee dmcs

// Local Variables:
// mode: C++
// End:
