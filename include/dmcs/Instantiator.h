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
 * @file   Instantitator.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Nov 28 2011 17:02
 *
 * @brief 
 *
 *
 */

#ifndef INSTANTIATOR_H
#define INSTANTIATOR_H

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "dmcs/Engine.h"
#include "dmcs/Evaluator.h"

namespace dmcs {

class Engine;
class Evaluator;

class Instantiator
{
public:
  Instantiator(const std::string& kbn, const EngineWPtr& e);

  ~Instantiator();

  EvaluatorPtr
  createEvaluator();

  void
  removeEvaluator(EvaluatorPtr eval);

private:
  void
  parseKB();

private:
  std::string kbname;
  TheoryPtr theory;
  EngineWPtr engine;
  std::list<EvaluatorPtr> evaluators;
};

typedef boost::shared_ptr<Instantiator> InstantiatorPtr;
typedef boost::weak_ptr<Instantiator> InstantiatorWPtr;

} // namespace dmcs

#endif // INSTANTIATOR_H

// Local Variables:
// mode: C++
// End:
