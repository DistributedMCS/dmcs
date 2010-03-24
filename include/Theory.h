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
 * @file   Theory.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Nov  8 11:03:46 2009
 * 
 * @brief  
 * 
 * 
 */

#include <map>
#include <vector>
#include <boost/bimap.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

#if !defined(THEORY_H)
#define THEORY_H

namespace dmcs {

typedef int Variable;

typedef std::vector<Variable> Variables;

typedef std::vector<Variable> Clause;
typedef boost::shared_ptr<Clause> ClausePtr;
typedef std::vector<ClausePtr> Theory;
typedef boost::shared_ptr<Theory> TheoryPtr;

///@todo TK: get rid of this guy
inline void
add(TheoryPtr& firstFormula,const TheoryPtr& otherFormula)
{
  firstFormula->insert(firstFormula->end(),
		  otherFormula->begin(),
		  otherFormula->end());
}

///@todo TK: get rid of this guy
inline  void
add(TheoryPtr& firstFormula,const ClausePtr& dis)
{
  firstFormula->push_back(dis);
}

///@todo TK: get rid of this guy or move it somewhere else, it does not belong here
inline TheoryPtr
createCNF(TheoryPtr& DNF, ClausePtr& clauseSoFar)
{
  TheoryPtr result(new Theory);

  ClausePtr currentClause = DNF->front();

  TheoryPtr remainingDNF(new Theory);

  Theory::iterator front = DNF->begin();
  advance(front, 1);
  std::back_insert_iterator<Theory> remDNFIt(*remainingDNF);
  std::copy(front,DNF->end(),remDNFIt);

  for(Clause::const_iterator it = currentClause->begin(); it != currentClause->end(); ++it)
    {
      ClausePtr newClause(new Clause);
      std::back_insert_iterator<Clause> newClauseIt(*newClause);
      std::copy(clauseSoFar->begin(),clauseSoFar->end(),newClauseIt);
      newClause->push_back(*it);
      if(remainingDNF->size() > 0)
	{
	  add(result,createCNF(remainingDNF,newClause));
	}
      else
	{
	  result->push_back(newClause);
	}
    }
  return result;
}

///@todo TK: are these guys still needed?
typedef std::map<Variable, std::string> MapId2Name;
typedef std::map<Variable, Variable> MapId2Lid;
typedef boost::bimap<Variable, std::string> BiMapId2Name;
typedef boost::bimap<Variable, Variable> BiMapId2Lid;
typedef std::vector<BiMapId2Name> BiMapsId2Name;
typedef std::vector<BiMapId2Lid> BiMapsId2Lid;

} // namespace dmcs

#endif // THEORY_H

// Local Variables:
// mode: C++
// End:
