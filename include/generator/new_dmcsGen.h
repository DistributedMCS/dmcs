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
 * @date   Fri Oct  26 22:11:26 2012
 * 
 * @brief Here stay all common methods used in generating test cases
 * for both static and dynamic dmcs.
 * 
 * 
 */

#ifndef __NEW_DMCS_GEN_H__
#define __NEW_DMCS_GEN_H__

#include "generator/ContextGenerator.h"
#include "mcs/BeliefTable.h"


namespace dmcs { namespace generator {

void
genBeliefTables(BeliefTableVecPtr sigma_vec, 
		std::size_t no_contexts, 
		std::size_t no_atoms);

void
genInterface(InterfaceVecPtr context_interfaces,
	     BeliefTableVecPtr context_interface_table_vec,
	     std::size_t no_contexts, 
	     std::size_t no_atoms, 
	     std::size_t no_interface_atoms,
	     BeliefTableVecPtr sigma_vec);

  } // namespace generator
} // dmcs

#endif // __NEW_DMCS_GEN_H__

// Local Variables:
// mode: C++
// End:
