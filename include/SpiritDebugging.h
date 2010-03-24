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
 * @file   SpiritDebugging.h
 * @author Peter Schüller
 * @date   Wed Mar 22 14:38:53 CET 2006
 * 
 * @brief  Recursive parse-tree print template function for boost::spirit
 */

#ifndef _SPIRIT_DEBUGGING_H_
#define _SPIRIT_DEBUGGING_H_

#include <string>
#include <ostream>

// boost::spirit parse-tree debugging
template<typename NodeT>
void printSpiritPT(std::ostream& o, const NodeT& node, const std::string& indent="");

template<typename NodeT>
void printSpiritPT(std::ostream& o, const NodeT& node, const std::string& indent)
{
  o << indent << "'" << std::string(node.value.begin(), node.value.end()) << "'\t\t\t(" << node.value.id().to_long() << ")" << std::endl;
  if( !node.children.empty() )
    {
      std::string cindent(indent + "  ");
      for(typename NodeT::const_tree_iterator it = node.children.begin(); it != node.children.end(); ++it)
	{
	  printSpiritPT(o, *it, cindent);
	}
    }
}

#endif // _SPIRIT_DEBUGGING_H_


// Local Variables:
// mode: C++
// End:
