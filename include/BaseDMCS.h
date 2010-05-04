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
 * @file   BaseDMCS.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Wed Nov  4 11:14:01 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_DMCS_H
#define BASE_DMCS_H


#include "BeliefState.h"
#include "Context.h"

namespace dmcs {

/**
 * @brief Base class for DMCS algorithms
 */
class BaseDMCS
{
private:
  
  // default ctor is private
  BaseDMCS();

protected:
  ContextPtr ctx; /// the context of DMCS
  TheoryPtr theory; /// the theory of DMCS

  /// ctor for children
  BaseDMCS(const ContextPtr& c, const TheoryPtr& t);

  /**
   * @par V interface
   *
   * @return list of local belief states
   */
  virtual BeliefStateListPtr
  localSolve(const BeliefStatePtr& V);

public:
  virtual
  ~BaseDMCS();
};

typedef boost::shared_ptr<BaseDMCS> DMCSPtr;

} // namespace dmcs

#endif // BASE_DMCS_H

// Local Variables:
// mode: C++
// End:
