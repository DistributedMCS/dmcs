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
 * @file   ReturnedBeliefState.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 13:25:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef RETURNED_BELIEF_STATE_H
#define RETURNED_BELIEF_STATE_H

#include <list>
#include <boost/shared_ptr.hpp>

#include "mcs/NewBeliefState.h"
#include "mcs/Printhelpers.h"

namespace dmcs {

struct ReturnedBeliefState : private ostream_printable<ReturnedBeliefState>
{
  ReturnedBeliefState()
    : belief_state(0), query_id(0)
  { }
  
  ReturnedBeliefState(NewBeliefState* bs, std::size_t qid)
    : belief_state(bs), query_id(qid)
  { }

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & belief_state;
    ar & query_id;
  }

  std::ostream&
  print(std::ostream& os) const;

  NewBeliefState* belief_state;
  std::size_t query_id;
};

typedef std::list<ReturnedBeliefState> ReturnedBeliefStateList;
typedef boost::shared_ptr<ReturnedBeliefStateList> ReturnedBeliefStateListPtr;

bool
my_compare(const ReturnedBeliefState& rbs1, const ReturnedBeliefState& rbs2);

bool
operator== (const ReturnedBeliefState& rbs1, const ReturnedBeliefState& rbs2);

void
remove_duplication(ReturnedBeliefStateListPtr& rbsl);
 
} // namespace dmcs

#endif // RETURNED_BELIEF_STATE_H

// Local Variables:
// mode: C++
// End:
