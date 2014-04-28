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
 * @file   DLVInstantitator.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan 24 2012 16:41
 *
 * @brief 
 *
 *
 */

#ifndef DLV_INSTANTIATOR_H
#define DLV_INSTANTIATOR_H

#include "dmcs/Instantiator.h"

namespace dmcs {

class DLVInstantiator : public Instantiator
{
public:
  static DLVInstantiatorPtr
  create(const EngineWPtr& e,
	 const std::string& kbspec);

  EvaluatorPtr
  createEvaluator(const InstantiatorWPtr& inst);

  void
  startThread(EvaluatorPtr eval,
	      std::size_t ctx_id,
	      BeliefTablePtr ex_sig,
	      NewConcurrentMessageDispatcherPtr md);

  void
  stopThread(EvaluatorPtr eval);

  std::string
  getKB();

private:
  DLVInstantiator(const EngineWPtr& e,
		  const std::string& kbspec);

private:
  std::string local_kb;
};

} // namespace dmcs 

#endif // DLV_INSTANTIATOR_H

// Local Variables:
// mode: C++
// End:
