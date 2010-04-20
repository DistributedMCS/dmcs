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
 * @file   BaseVisitor.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:36:27 2009
 * 
 * @brief  
 * 
 * 
 */

#include "Theory.h"
#include "Signature.h"

#ifndef BASE_VISITOR_H_
#define BASE_VISITOR_H_

namespace dmcs {


/**
 * @brief Base class for visitors.
 */
class BaseVisitor
{
public:
  virtual 
  ~BaseVisitor()
  { }
  
  virtual void
  visitTheory(const TheoryPtr t, std::size_t size) = 0;
  
  virtual void
  visitClause(const ClausePtr c) = 0;
};

} // namespace dmcs

#endif // _MCS_BASE_VISITOR_H_

// Local Variables:
// mode: C++
// End:
