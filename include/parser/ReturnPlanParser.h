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
 * @file   ReturnPlanParser.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Oct  22 16:25:24 2012
 * 
 * @brief  
 * 
 * 
 */



#ifndef __RETURN_PLAN_PARSER__INCLUDED__
#define __RETURN_PLAN_PARSER__INCLUDED__

#include "mcs/NewBeliefState.h"

#include <istream>
#include <string>

namespace dmcs
{

class ReturnPlanParser
{
public:
  static ReturnPlanMapPtr parseStream(std::istream& in);
  static ReturnPlanMapPtr parseFile(const std::string& infile);
  static ReturnPlanMapPtr parseString(const std::string& instr);
};

} // namespace dmcs

#endif // __RETURN_PLAN_PARSER__INCLUDED__

// Local Variables:
// mode: C++
// End:
