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
 * @file   DLVInstantitator.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan 24 2012 16:49
 *
 * @brief 
 *
 *
 */

#include "dmcs/DLVInstantiator.h"
#include "dmcs/DLVEvaluator.h"
#include "dmcs/InputProvider.h"

namespace dmcs {

DLVInstantiator::DLVInstantiator(const EngineWPtr& e,
				 const std::string& kbspec)
  : Instantiator(e, kbspec)
{
  InputProvider inp;
  inp.addFileInput(kbspec);
  
  std::istream& iss = inp.getAsStream();
  local_kb = "";
  while (!iss.eof())
    {
      std::string line;
      std::getline(iss, line);
      if (line != "")
	{
	  local_kb = local_kb + line + "\n";
	}
    }
}


DLVInstantiatorPtr
DLVInstantiator::create(const EngineWPtr& e, const std::string& kbspec)
{
  DLVInstantiatorPtr dlv_inst(new DLVInstantiator(e, kbspec));
  return dlv_inst;
}


EvaluatorPtr
DLVInstantiator::createEvaluator(const InstantiatorWPtr& inst)
{
  InstantiatorPtr inst_p = inst.lock();
  Instantiator* inst_s = inst_p.get();
  assert (this == inst_s);

  EvaluatorPtr eval(new DLVEvaluator(inst));
  evaluators.push_back(eval);

  boost::thread* t = NULL;
  eval_threads.push_back(t);

  return eval;
}


void
DLVInstantiator::startThread(EvaluatorPtr eval,
			     std::size_t ctx_id,
			     BeliefTablePtr ex_sig,
			     NewConcurrentMessageDispatcherPtr md)
{
  //DLVEvaluatorPtr dlv_eval = boost::static_pointer_cast<DLVEvaluator>(eval);
  eval->init_mqs(md);

  EvaluatorWrapper eval_wrapper;
  boost::thread*& t = getThread(eval);
  t = new boost::thread(eval_wrapper, eval, ctx_id, ex_sig, md);
}


std::string
DLVInstantiator::getKB()
{
  return local_kb;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
