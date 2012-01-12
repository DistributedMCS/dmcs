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
 * @file   DLVResultParser.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  12 12:20:40 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef DLV_RESULT_PARSER_H
#define DLV_RESULT_PARSER_H

#include "mcs/NewBeliefState.h"
#include "mcs/BeliefTable.h"

namespace dmcs {

class DLVResultParser
{
public:
  typedef boost::function<void (BeliefState*)> BeliefStateAdder;

  DLVResultParser(BeliefTablePtr b);

  void
  parse(std::istream& is, BeliefStateAdder bsAdder);

protected:
  BeliefTablePtr btab;
};

} // namespace dmcs

#endif // DLV_RESULT_PARSER_H


// Local Variables:
// mode: C++
// End:
