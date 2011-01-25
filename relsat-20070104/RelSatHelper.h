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
 * @file   RelSatHelper.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  17 16:46:12 2011
 * 
 * @brief  
 * 
 * 
 */

#include "Clause.h"
#include "ClauseList.h"
#include "SATInstance.h"
#include "SATSolver.h"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#ifndef RELSAT_HELPER
#define RELSAT_HELPER


inline std::istream&
operator>> (std::istream& is, ClauseList& cl)
{
  typedef boost::char_separator<char> separator;
  typedef boost::tokenizer<separator> tokenizer;

  std::string s;
  std::getline(is, s);

  while (!s.empty())
    {
      std::size_t eMaxVar = 0;
      std::vector<VariableID> atoms;

      separator sep(" ");
      tokenizer tok(s, sep);

      // First travel through the clause just to collect the atoms and find out eMaxVar
      int atom;
      for (tokenizer::const_iterator it = tok.begin(); it != tok.end(); ++it)
	{
	  atom = boost::lexical_cast<VariableID>(*it);
	  atoms.push_back(atom);
	  if (eMaxVar < std::abs(atom))
	    {
	      eMaxVar = std::abs(atom);
	    }
	}
      
      VariableSet xPositiveVariables(eMaxVar);
      VariableSet xNegativeVariables(eMaxVar);
      
      for (std::vector<VariableID>::const_iterator it = atoms.begin(); it != atoms.end(); ++it)
	{
	  atom = *it;
	  
	  assert (atom != 0);
	  
	  if (atom > 0)
	    {
	      xPositiveVariables.vAddVariable(atom-1);
	    }
	  else
	    {
	      xNegativeVariables.vAddVariable(0-(atom+1));
	    }
	}

      assert(xNegativeVariables.iCount() + xPositiveVariables.iCount() > 0);

      Clause* c = new Clause((VariableList&)xPositiveVariables, 
			     (VariableList&)xNegativeVariables,
			     1);

      cl.vAddClause(c);
      std::getline(is, s);
    }

  return is;
}


inline int
back_2_lit(int var)
{
  int tmp = Clause::iGetVariable(var);
  if (var & 0x80000000)
    {
      return (-tmp - 1);
    }

  return (tmp + 1);
}


inline std::ostream&
operator<< (std::ostream& os, const VariableList& vl)
{
  for (int i = 0; i < vl.iCount(); ++i)
    {
      os << back_2_lit(vl.iVariable(i)) << " ";
    }

  return os;
}



inline std::ostream&
operator<< (std::ostream& os, const Clause& c)
{
  for (int i = 0; i < c.iVariableCount(); ++i)
    {
      os << back_2_lit(c.eConstrainedLiteral(i)) << " ";
    }

  return os;
}



inline std::ostream&
operator<< (std::ostream& os, const ClauseList& cls)
{
  for (int i = 0; i < cls.iClauseCount(); ++i)
    {
      Clause* cl = cls.pClause(i);
      os << *cl << std::endl;
    }

  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const SATInstance& si)
{
  for (int i = 0; i < si.iClauseCount(); ++i)
    {
      Clause* cl = si.pClause(i);
      os << *cl << std::endl;
    }

  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const VariableStruct& vs)
{
  os << "xPositiveClauses: " << vs.xPositiveClauses << std::endl;
  os << "xNegativeClauses: " << vs.xNegativeClauses << std::endl;
  if (vs.pReason)
    {
      os << "pReason:          " << *vs.pReason << std::endl;
    }
  else
    {
      os << "pReason:          NULL" << std::endl;
    }

  if (vs.pDeleteReason)
    {
      os << "pDeleteReason:    " << *vs.pDeleteReason << std::endl;
    }
  else
    {
      os << "pDeleteReason:    NULL" << std::endl;
    }

  os << "xUnitClause:      " << vs.xUnitClause << std::endl;
  os << "xDeleteList:      " << vs.xDeleteList << std::endl;

  if (vs.bBranch)
    {
      os << "bBranch:          1" << std::endl;
    }
  else
    {
      os << "bBranch:          0" << std::endl;
    }
  //os << "pSolutionInfo:          " << *vs.pSolutionInfo << std::endl;

  return os;
}

#endif

// Local Variables:
// mode: C++
// End:
