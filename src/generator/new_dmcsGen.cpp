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
 * @file   dmcsGen.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Oct  26 22:08:21 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <algorithm>
#include <iostream>
#include <sstream>

#include "generator/new_dmcsGen.h"
//#include "dmcs/Log.h"

namespace dmcs { namespace generator {

void
genBeliefTables(BeliefTableVecPtr sigma_vec, std::size_t no_contexts, std::size_t no_atoms)
{
  std::stringstream ctx_id;
  std::stringstream atm_id;
  std::string atom_name;

  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      ctx_id.str("");
      ctx_id << i;
      
      BeliefTablePtr btab(new BeliefTable);
      Belief belief_epsilon(i, "epsilon");
      btab->storeAndGetID(belief_epsilon);

      // create local BeliefTable for context i
      for (std::size_t j = 0; j < no_atoms; ++j)
	{
	  atm_id.str("");
	  atm_id << j+1;

	  // atom name of the form: context_contextid, then atom_atomid
	  // ciaj for short
	  // e.g., c1a2
	  atom_name = "c" + ctx_id.str() + "a" + atm_id.str();
	  Belief belief(i, atom_name);
	  btab->storeAndGetID(belief);
	}
      sigma_vec->push_back(btab);
    }
}



void
genInterface(InterfaceVecPtr context_interfaces, 
	     BeliefTableVecPtr context_interface_table_vec,
	     std::size_t no_contexts, 
	     std::size_t no_atoms,
	     std::size_t no_interface_atoms,
	     BeliefTableVecPtr sigma_vec)
{
  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      Interface c_i_interface;
      BeliefTablePtr local_sig = (*sigma_vec)[i];
      BeliefTablePtr c_i_interface_table(new BeliefTable);

      for (std::size_t j = 0; j < no_interface_atoms; ++j)
	{
	  while (1)
	    {
	      std::size_t k = (rand() % no_atoms) + 1;
	      if (std::find(c_i_interface.begin(), c_i_interface.end(), k) == c_i_interface.end())
		{
		  c_i_interface.push_back(k);
		  break;
		}
	    };
	}

      // storeWithID must be done with increasing IDs; hence this additional loop
      std::sort(c_i_interface.begin(), c_i_interface.end());
      for (Interface::const_iterator it = c_i_interface.begin(); it != c_i_interface.end(); ++it)
	{
	  ID id = ID(i | ID::MAINKIND_BELIEF, *it);
	  const Belief& b = local_sig->getByID(id);
	  c_i_interface_table->storeWithID(b, id);
	}

      context_interfaces->push_back(c_i_interface);
      context_interface_table_vec->push_back(c_i_interface_table);
    }
}

  } // namespace generator
} // namespace dmcs
