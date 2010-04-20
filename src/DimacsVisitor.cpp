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
 * @file   DimacsVisitor.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 11:15:24 2009
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "DimacsVisitor.h"
#include <set>
#include <iostream> 

using namespace dmcs;


DimacsVisitor::DimacsVisitor(std::ostream& s)
  : stream(s)
{ }

std::ostream&
DimacsVisitor::getStream()
{
  return stream;
}

void
DimacsVisitor::visitTheory1(const TheoryPtr t)
{
   for (Theory::const_iterator it = t->begin(); it != t->end(); ++it)
    {
      visitClause(*it);
    }
}

void
DimacsVisitor::visitTheory(const TheoryPtr t, std::size_t size)
{
  stream << "p cnf " << size << " " << t->size() << std::endl; 
  visitTheory1(t);
}

void
DimacsVisitor::visitClause(const ClausePtr c)
{
  for(Clause::iterator kt = c->begin(); kt != c->end(); ++kt)
    {
      stream << *kt << " " ;	    	    
    } 
  stream << "0" << std::endl;
}
