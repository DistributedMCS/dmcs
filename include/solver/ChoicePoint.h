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
 * @file   ChoicePoint.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Feb  1 17:56:42 2011
 * 
 * @brief  
 * 
 * 
 */


#include "mcs/BeliefState.h"

#include <stack>
#include <boost/shared_ptr.hpp>

#ifndef CHOICE_POINT_H
#define CHOICE_POINT_H


namespace dmcs {

struct ChoicePoint 
{
  PartialBeliefState* input;
  Decisionlevel* decision;

  ChoicePoint(PartialBeliefState* i, Decisionlevel* d)
    : input(i), decision(d)
  { }
};

typedef boost::shared_ptr<ChoicePoint> ChoicePointPtr;
typedef std::stack<ChoicePointPtr> Trail;
typedef boost::shared_ptr<Trail> TrailPtr;

inline std::ostream&
operator<< (std::ostream& os, const ChoicePoint& cp)
{
  os << *(cp.input) << std::endl << *(cp.decision);

  return os;
}


inline ConflictNotification*
getNextFlip(ChoicePointPtr& cp, VecSizeTPtr& orig_sigs_size)
{
  PartialBeliefState* input = cp->input;
  Decisionlevel* decision = cp->decision;

  std::size_t atom_gid = decision->last(); // global id of the atom

  assert (atom_gid <= decision->getGlobalSigSize());
 
  VecSizeT::const_iterator it = orig_sigs_size->begin();
  PartialBeliefState::const_iterator jt = input->begin();
  std::size_t pos = 0;
  while (atom_gid > (*it))
    {
      atom_gid -= (*it);
      ++it;
      ++jt;
      ++pos;
    }

  // Now, atom_gid is the position of the bit representing the atom
  // check for safety
  assert (testBeliefSet(*jt, atom_gid) != PartialBeliefSet::DMCS_UNDEF);

  // get the next on bit
  std::size_t next_atom = jt->state_bit.get_next(atom_gid);

  if (next_atom == 0)
    {
      return 0;
    }

  // build partial belief state and return. For now it looks wasteful
  // as we set up only 1 atom but use the whole PartialBeliefState,
  // but later when we transfer the decision level back and ford, this
  // should help with setting up more than 1 atoms.
  const std::size_t system_size = orig_sigs_size->size();
  PartialBeliefState* partial_ass = new PartialBeliefState(system_size, PartialBeliefSet());
  
  PartialBeliefState::iterator pa_it = partial_ass->begin();
  std::advance(pa_it, pos);

  // set the oposite value of the corresponding assignment in input.
  if (testBeliefSet(*jt, next_atom) == PartialBeliefSet::DMCS_TRUE)
    {
      setBeliefSet(*pa_it, next_atom, PartialBeliefSet::DMCS_FALSE);
    }
  else
    {
      setBeliefSet(*pa_it, next_atom);
    }

  decision->pop_back();
  decision->setDecisionlevel(next_atom);
  ConflictNotification* cn = new ConflictNotification(0, partial_ass, decision);

  return cn;
}

} // namespace dmcs

#endif // CHOICE_POINT_H

// Local Variables:
// mode: C++
// End:
