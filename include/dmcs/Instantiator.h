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

#include "dmcs/AbstractContext.h"
#include "dmcs/Engine.h"
#include "dmcs/Evaluator.h"
#include "dmcs/InputProvider.h"

namespace dmcs {

class Engine;
class Evaluator;

class Instantiator
{
public:
  //static InstantiatorPtr
  //create(const std::string& kbspec, const EngineWPtr& e);

  virtual
  ~Instantiator();
  
  virtual EvaluatorPtr
  createEvaluator(const InstantiatorWPtr& inst,
		  const NewConcurrentMessageDispatcherPtr md) = 0;

  // call this with a weak pointer to unregister the instantiator
  // (see comment in Engine.h for why this must be a weak pointer)
  virtual void
  removeEvaluator(EvaluatorWPtr eval);

  std::istream&
  getKB();

  std::size_t
  getNoEvaluators();

protected:
  Instantiator(const EngineWPtr& e,
	       const std::string& kbn);

protected:
  std::list<EvaluatorPtr> evaluators;

private:
  EngineWPtr engine;

  // knowledge base specifier (can be filename or sql database connection string or rdf tuplestore location IRI)
  std::string kbspec;

  InputProvider inp;

  //TheoryPtr theory; // remove -> we can let this unspecified here, derived classes
  //may have various methods to represent this, think of an sql context, it will have
  //no theory, kbspec will be a connection string, the theory will be in the database
  //running on some host
};

} // namespace dmcs

#endif // INSTANTIATOR_H

// Local Variables:
// mode: C++
// End:
