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
 * @file   DimacsVisitor.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:57:55 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef DIMACS_VISITOR_H
#define DIMACS_VISITOR_H


#include "BaseVisitor.h"
#include "Signature.h"

#include <iosfwd>


namespace dmcs {

class DimacsVisitor : public BaseVisitor
{
protected:
  std::ostream& stream;

public:
  explicit
  DimacsVisitor(std::ostream&);

  /**
   * returns the stream of the visitor.
   */
  std::ostream&
  getStream();

  ///@todo use std::size_t no_vars (== local_sig_size + neighbor_vars + V_vars) instead of whole sig
  void 
  visitTheory(const TheoryPtr t, std::size_t size);

  /// @todo TK: clean up!
  void 
  visitTheory1(const TheoryPtr t);

  void
  visitClause(const ClausePtr c);

};


} // namespace dmcs

#endif // DIMACS_VISITOR_H

// Local Variables:
// mode: C++
// End:
