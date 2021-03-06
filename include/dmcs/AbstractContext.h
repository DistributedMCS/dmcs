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
 * @file   AbstractContext.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Nov 16:37:24 29 2011
 *
 * @brief 
 *
 *
 */

#ifndef ABSTRACT_CONTEXT_H
#define ABSTRACT_CONTEXT_H

#include <list>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace dmcs {

class Engine;
class Instantiator;
class Evaluator;

class DLVEngine;
class DLVInstantiator;
class DLVEvaluator;

typedef boost::shared_ptr<Engine> EnginePtr;
typedef boost::weak_ptr<Engine> EngineWPtr;

typedef boost::shared_ptr<Instantiator> InstantiatorPtr;
typedef boost::weak_ptr<Instantiator> InstantiatorWPtr;

typedef boost::shared_ptr<Evaluator> EvaluatorPtr;
typedef boost::weak_ptr<Evaluator> EvaluatorWPtr;
typedef std::list<EvaluatorPtr> EvaluatorList;

typedef boost::shared_ptr<DLVEngine> DLVEnginePtr;
typedef boost::weak_ptr<DLVEngine> DLVEngineWPtr;

typedef boost::shared_ptr<DLVInstantiator> DLVInstantiatorPtr;
typedef boost::weak_ptr<DLVInstantiator> DLVInstantiatorWPtr;

typedef boost::shared_ptr<DLVEvaluator> DLVEvaluatorPtr;
typedef boost::weak_ptr<DLVEvaluator> DLVEvaluatorWPtr;

typedef std::list<boost::thread*> BoostThreadList;

} // namespace dmcs

#endif // ABSTRACT_CONTEXT_H

// Local Variables:
// mode: C++
// End:
