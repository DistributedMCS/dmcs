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
 * @file   MultipleRingQPGenerator.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Tue Jan 21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef MULTIPLE_RING_QP_GENERATOR_H_
#define MULTIPLE_RING_QP_GENERATOR_H_

#include "Context.h"
#include "QueryPlanGenerator.h"

namespace dmcs { namespace generator {


class MultipleRingQPGenerator : public QueryPlanGenerator
{
public:
  MultipleRingQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : QueryPlanGenerator(contexts_, query_plan_)
  { }

  void
  genNeighbors(const ContextPtr& context)
  {
    std::size_t id = context->getContextID();
    std::size_t earLoopSize = (context->getSystemSize() + 3) / 3;
    std::size_t secondLoopJump = earLoopSize-1;
    std::size_t firstLoopEnd = 2 +earLoopSize + earLoopSize -3;
    if ((std::size_t)id == 2) 
      {
	genNeighbor(context, id+1);
	genNeighbor(context, id+earLoopSize);
	return;
      }

    if ((std::size_t)id == (earLoopSize +1 )) 
      {
	genNeighbor(context, 1);
	genNeighbor(context, id+secondLoopJump);
	return;
      }
    if ((std::size_t)id == context->getSystemSize()) 
      {
	genNeighbor(context, earLoopSize);
	return;
      }

    if ((std::size_t)id == firstLoopEnd) 
      {
	genNeighbor(context, 3);
	return;
      }

    genNeighbor(context, id+1);

  }

  void
  create_interface(const ContextPtr& context)
  { }
};

  } // namespace generator
} // namespace dmcs

#endif // MULTIPLE_RING_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
