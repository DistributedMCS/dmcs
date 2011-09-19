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
 * @file   Solver.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Sep  18 12:16:20 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef SOLVER_H
#define SOLVER_H

class Solver
{
public:
  Solver(KBType kb, ConcurrentMessageQueue iq, ConcurrentMessageQueue oq)
    : input_queue(iq), output_queue(oq)
  { }

  // evaluate in order to return from k1-th to k2-th models
  void
  solve(std::size_t k1, std::size_t k2)
  { }

private:
  KBType kb;
  ConcurrentMessageQueue input_queue;
  ConcurrentMessageQueue output_queue;
};

#endif // SOLVER_H

// Local Variables:
// mode: C++
// End:
