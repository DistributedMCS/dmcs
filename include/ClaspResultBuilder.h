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
 * @file   ClaspResultBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:38:22 2009
 * 
 * @brief  
 * 
 * 
 */



#ifndef CLASP_RESULT_BUILDER_H
#define CLASP_RESULT_BUILDER_H

#include "BaseBuilder.h"
#include "BeliefState.h"
#include "Context.h"
#include "Theory.h"

#include <boost/functional/hash.hpp>

namespace dmcs {

template<class Grammar>
class ClaspResultBuilder : public BaseBuilder<Grammar>
{
public:
  ClaspResultBuilder(const SignatureByLocal&, const BeliefStateListPtr&, std::size_t system_size);

  void
  buildNode (typename BaseBuilder<Grammar>::node_t& node);
  
private:
  void
  add_literal(typename BaseBuilder<Grammar>::node_t& node, BeliefStatePtr& bs);


  const SignatureByLocal& local_sig;

  std::size_t system_size;

  BeliefStateListPtr belief_states;
};

} // namespace dmcs

#endif // CLASP_RESULT_BUILDER_H

#include "ClaspResultBuilder.tcc"

// Local Variables:
// mode: C++
// End:
