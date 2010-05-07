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
 * @file   DLVVisitor.cpp
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Fri Jan 29 2010 00:25
 * 
 * @brief
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "DLVVisitor.h"
#include <set>
#include <iostream> 

using namespace dmcs;


DLVVisitor::DLVVisitor(std::ostream& s)
  : LogicVisitor(s)
{ }

void
DLVVisitor::printPositiveBridgeRule(std::ostream& os,const BridgeAtom& b)
{
  os << (char)(b.second+'a')<< "" << b.first;
}

void
DLVVisitor::printNegativeBridgeRule(std::ostream& os,const BridgeAtom& b)
{
  os << "not " << (char)(b.second+'a')<< "" << b.first;
}
