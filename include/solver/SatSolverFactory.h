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
 * @file   SatSolverFactory.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan 4 17:49:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef SAT_SOLVER_FACTORY_H
#define SAT_SOLVER_FACTORY_H

#include "RelSatSolver.h"



namespace dmcs {


class SatSolverFactory
{
public:
  SatSolverFactory(bool il,
		   std::size_t mid,
		   std::size_t sid,
		   const TheoryPtr& t,
		   const SignaturePtr& ls,
		   const HashedBiMap* co,
		   std::size_t ss,
		   MessagingGatewayBC* m)
    : is_leaf(il),
      my_id(mid),
      session_id(sid),
      theory(t),
      local_sig(ls),
      c2o(co),
      system_size(ss),
      mg(m)
  { }

  template<typename aSatSolverTypePtr>
  aSatSolverTypePtr
  create();

private:
  bool                           is_leaf;
  std::size_t                    my_id;
  std::size_t                    session_id;
  const TheoryPtr                theory;
  const SignaturePtr             local_sig;
  const HashedBiMap*             c2o;
  std::size_t                    system_size;
  MessagingGatewayBC*            mg;
};


template<>
inline RelSatSolverPtr
SatSolverFactory::create<RelSatSolverPtr>()
{
  RelSatSolverPtr relsatsolver(new RelSatSolver(is_leaf,
						my_id, session_id,
						theory, local_sig, 
						c2o, system_size, 
						mg));

  return relsatsolver;
}

} // namespace dmcs

#endif

// Local Variables:
// mode: C++
// End:
