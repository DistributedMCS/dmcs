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
 * @file   dmcsGen.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Aug  30 13:48:00 2010
 * 
 * @brief Here stay all common methods used in generating test cases
 * for both static and dynamic dmcs.
 * 
 * 
 */

#ifndef DMCS_GEN_H
#define DMCS_GEN_H

#include "Signature.h"
#include "generator/ContextGenerator.h"

#define LP_EXT  ".lp"
#define BR_EXT  ".br"

namespace dmcs { namespace generator {

typedef std::vector<std::size_t> Interface;
typedef std::vector<Interface> InterfaceVec;
typedef boost::shared_ptr<InterfaceVec> InterfaceVecPtr;

void
genSignatures(SignatureVecPtr sigmas, std::size_t no_contexts, std::size_t no_atoms);

void
genInterface(InterfaceVecPtr context_interfaces, std::size_t no_contexts, 
	     std::size_t no_atoms, std::size_t no_interface_atoms);

  } // namespace generator
} // dmcs

#endif // DMCS_GEN_H

// Local Variables:
// mode: C++
// End:
