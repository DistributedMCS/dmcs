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
 * @file   ReturnedBeliefState.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 13:45:27 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/ReturnedBeliefState.h"

namespace dmcs {

std::ostream&
ReturnedBeliefState::print(std::ostream& os) const
{						
  if (belief_state)
    {
      os << *belief_state << ", qid = " << query_id;
    }
  else
    {
      os << "NULL, qid = " << query_id;
    }
  
  return os;
}



bool
my_compare(const ReturnedBeliefState* rbs1, const ReturnedBeliefState* rbs2)
{
  if (!rbs1->belief_state)
    {
      return false;
    }

  if (!rbs2->belief_state)
    {
      return true;
    }

  if (*(rbs1->belief_state) < *(rbs2->belief_state))
    {
      return true;
    }
  else if (*(rbs1->belief_state) == *(rbs2->belief_state))
    {
      if (rbs1->query_id < rbs2->query_id)
	{
	  return true;
	}
    }
  
  return false;
}



bool
operator== (const ReturnedBeliefState& rbs1, const ReturnedBeliefState& rbs2)
{
  if (!rbs1.belief_state && !rbs2.belief_state)
    {
      return true;
    }

  if (!rbs1.belief_state || !rbs2.belief_state)
    {
      return false;
    }

  return ((*(rbs1.belief_state) == *(rbs2.belief_state)) 
	  && (rbs1.query_id == rbs2.query_id));
}



// only for sorted list, hence O(n)
// deep removing: also delete pointers
void
remove_duplication(ReturnedBeliefStateListPtr& rbsl)
{
  ReturnedBeliefStateList::iterator first = rbsl->begin();
  ReturnedBeliefStateList::iterator last = rbsl->end();

  ReturnedBeliefStateList::iterator mid = first;
  ReturnedBeliefStateList::iterator beg_remove = first;
  beg_remove++;

  while (++first != last)
  {
    if (!(**mid == **first))
      {
	for (ReturnedBeliefStateList::iterator it = beg_remove; it != first; ++it)
	  {
	    ReturnedBeliefState* rbs = *it;
	    NewBeliefState* pbs = rbs->belief_state;
	    delete pbs;
	    pbs = 0;
	  }

	// copy here, but we can afford it because it contains just 2 integers
	**(++mid) = **first;
	beg_remove = first;
	beg_remove++;
      }
  }

  for (ReturnedBeliefStateList::iterator it = beg_remove; it != last; ++it)
    {
      ReturnedBeliefState* rbs = *it;
      NewBeliefState* pbs = rbs->belief_state;
      delete pbs;
      pbs = 0;      
    }

  std::size_t d = std::distance(rbsl->begin(), mid);
  rbsl->resize(d+1);
}


 
} // namespace dmcs

// Local Variables:
// mode: C++
// End:
