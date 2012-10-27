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
 * @file   BeliefTable.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefTable.h"

#include <sstream>

#include <boost/tokenizer.hpp>

namespace dmcs {

BeliefTable::~BeliefTable()
{
}

// print without relying on random access address index!
std::ostream& BeliefTable::print(std::ostream& o) const
{
  ReadLock lock(mutex);

  const AddressIndex& aidx = container.get<impl::AddressTag>();
  for(AddressIndex::const_iterator it = aidx.begin();
      it != aidx.end(); ++it)
    {
      o << "  " << ID(it->kind, it->address) << ":" << *it << std::endl;
    }
  return o;
}


std::string
BeliefTable::gen_print() const
{
  std::string tmp = "{\n";

  boost::char_separator<char> sep(",()");

  const AddressIndex& aidx = container.get<impl::AddressTag>();
  for (AddressIndex::const_iterator it = aidx.begin(); it != aidx.end(); ++it)
    {
      Belief b = *it;
      if (b.address != 0)
	{
	  std::stringstream out;
	  out << b.address;
	  tmp = tmp + "  " + out.str() + ": [";

	  boost::tokenizer<boost::char_separator<char> > tok(b.text, sep);
	  boost::tokenizer<boost::char_separator<char> >::const_iterator it = tok.begin();
	  tmp = tmp + *it;
	  ++it;

	  for (; it != tok.end(); ++it) tmp = tmp + ", " + *it;

	  tmp = tmp + "],\n";
	}
    }

  tmp = tmp + "},\n";
  return tmp;
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
