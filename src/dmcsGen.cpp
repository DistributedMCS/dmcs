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
 * @date   Mon Aug  30 10:38:00 2010
 * 
 * @brief  
 * 
 * 
 */

#include <sstream>

#include "generator/dmcsGen.h"

namespace dmcs { namespace generator {

void
genSignatures(SignatureVecPtr sigmas, std::size_t no_contexts, std::size_t no_atoms)
{
  std::stringstream out;
  std::string atom_name;

  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      out.str("");
      out << i;
      SignaturePtr s(new Signature);

      // create local Signature for context i
      for (std::size_t j = 0; j < no_atoms; ++j)
	{
	  atom_name = (char)(j + 'a') + out.str();
	  s->insert(Symbol(atom_name, i, j+1, j+1));
	}
      sigmas->push_back(s);
    }
}



void
genInterface(InterfaceVec& context_interfaces, 
	     std::size_t no_contexts, 
	     std::size_t no_interface_atoms)
{
  for (std::size_t i = 1; i <= no_contexts; ++i)
    {
      Interface c_i_interface;

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
      context_interfaces.push_back(c_i_interface);
    }

#ifdef DEBUG
  std::cerr << "Interface of contexts:" << std::endl;
  for (std::size_t i = 0; i < no_contexts; ++i)
    {
      std::cerr << "C_" << i+1 << ": ";
      Interface ci = context_interfaces[i];
      for (std::size_t j = 0; j < no_interface_atoms; ++j)
	{
	  std::cerr << ci[j] << "=" << (char)(ci[j] +'a' - 1) << " ";
	}
      std::cerr << std::endl;
    }
#endif // DEBUG  

}

  } // namespace generator
} // namespace dmcs
