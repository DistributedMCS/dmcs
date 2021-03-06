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
  Heads(NewBeliefState* h,
	std::size_t k_one,
	std::size_t k_two)
    : heads(h),
      k1(k_one),
      k2(k_two)
  { }

  NewBeliefState*
  getHeads()
  {
    return heads;
  }

  std::size_t
  getK1()
  {
    return k1;
  }

  std::size_t
  getK2()
  {
    return k2;
  }

  bool
  operator==(const Heads& h) const
  {
    if (heads == NULL)
      {
	if (h.heads == NULL)
	  {
	    return true;
	  }
	else
	  {
	    return false;
	  }
      }
    else
      {
	if (h.heads == NULL)
	  {
	    return false;
	  }
	else
	  {
	    return (*heads == *(h.heads) && k1 == h.k1 && k2 == h.k2);
	  }
      }
  }


  bool
  operator!=(const Heads& h) const
  {
    if (heads == NULL)
      {
	if (h.heads == NULL)
	  {
	    return false;
	  }
	else
	  {
	    return true;
	  }
      }
    else
      {
	if (h.heads == NULL)
	  {
	    return true;
	  }
	else
	  {
	    return (*heads != *(h.heads) || k1 != h.k1 || k2 != h.k2);
	  }
      }
  }

protected:
  NewBeliefState* heads;
  std::size_t k1;
  std::size_t k2;
};


class HeadsPlusBeliefState : public Heads
{
public:
  HeadsPlusBeliefState(const NewBeliefState* i,
		       NewBeliefState* h, 
		       std::size_t k1,
		       std::size_t k2)
    : Heads(h, k1, k2), input(i)
  { }

  const NewBeliefState*
  getInputBeliefState() const
  {
    return input;
  }

private:
  const NewBeliefState* input; // the input belief state from that this head is inferred (by BridgeRuleEvaluator)
};

typedef std::pair<Heads*, NewBeliefState*> HeadsBeliefStatePair;

} // namespace dmcs

#endif // HEADS_H

// Local Variables:
// mode: C++
// End:
