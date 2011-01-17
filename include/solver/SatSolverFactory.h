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
		   std::size_t my_id_,
		   const TheoryPtr& theory_,
		   //const ProxySignatureByLocalPtr mixed_sig_,
		   const SignaturePtr& local_sig_,
		   const BeliefStatePtr& localV_,
		   std::size_t system_size_,
		   MessagingGatewayBCPtr& mg_,
		   const ConcurrentMessageQueuePtr& dsn,
		   const ConcurrentMessageQueuePtr& srn)
    : is_leaf(il),
      my_id(my_id_),
      theory(theory_),
      //      mixed_sig(mixed_sig_),
      local_sig(local_sig_),
      localV(localV_),
      system_size(system_size_),
      mg(mg_), 
      dmcs_sat_notif(dsn), 
      sat_router_notif(srn)
  { }

  template<typename aSatSolverTypePtr>
  aSatSolverTypePtr
  create();

private:
  bool                           is_leaf;
  std::size_t                    my_id;
  const TheoryPtr                theory;
  const SignaturePtr             local_sig;
  const BeliefStatePtr           localV;
  //const ProxySignatureByLocalPtr mixed_sig;
  std::size_t                    system_size;
  MessagingGatewayBCPtr          mg;
  ConcurrentMessageQueuePtr      dmcs_sat_notif;
  ConcurrentMessageQueuePtr      sat_router_notif;
};


template<>
inline RelSatSolverPtr
SatSolverFactory::create<RelSatSolverPtr>()
{
  RelSatSolverPtr relsatsolver(new RelSatSolver(is_leaf, my_id, 
						theory, local_sig, 
						localV, system_size, 
						mg, dmcs_sat_notif, 
						sat_router_notif));

  return relsatsolver;
}

} // namespace dmcs

#endif

// Local Variables:
// mode: C++
// End:
