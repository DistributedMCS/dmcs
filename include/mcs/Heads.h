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
 * @file   Heads.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Feb  16 16:46:16 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef HEADS_H
#define HEADS_H

#include "mcs/NewBeliefState.h"

namespace dmcs {

class Heads
{
public:
  Heads(NewBeliefState* h)
    : heads(h)
  { }

  NewBeliefState*
  getHeads()
  {
    return heads;
  }

protected:
  NewBeliefState* heads;
};


class HeadsPlusBeliefState : public Heads
{
public:
  HeadsPlusBeliefState(NewBeliefState* h, const NewBeliefState* i)
    : Heads(h), input(i)
  { }

  const NewBeliefState*
  getInputBeliefState() const
  {
    return input;
  }

private:
  const NewBeliefState* input; // the input belief state from that this head is inferred (by BridgeRuleEvaluator)
};

} // namespace dmcs

#endif // HEADS_H

// Local Variables:
// mode: C++
// End:
