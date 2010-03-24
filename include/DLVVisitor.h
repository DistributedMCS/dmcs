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
 * @file   DLVVisitor.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Fri Jan 29 2010 00:23
 * 
 * @brief  
 * 
 * 
 */


#ifndef DLV_VISITOR_H
#define DLV_VISITOR_H

#include "LogicVisitor.h"
#include "Context.h"
#include "Rule.h"

#include <iosfwd>

namespace dmcs {

class DLVVisitor : public LogicVisitor
{

public:
  explicit
  DLVVisitor(std::ostream&);

  void
  printPositiveBridgeRule(std::ostream& os,const BridgeAtom& b);
   
  void
  printNegativeBridgeRule(std::ostream& os,const BridgeAtom& b);

};

} // namespace dmcs

#endif // DLV_VISITOR_H

// Local Variables:
// mode: C++
// End:
