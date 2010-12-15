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
 * @file   Rule.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  30 15:06:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef RULE_H
#define RULE_H

#include <iterator>
#include <list>
#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "Variable.h"




namespace dmcs {

typedef std::size_t Atom;
typedef std::ptrdiff_t Literal;
typedef std::list<Atom> Head;
typedef std::list<Atom> PositiveBody;
typedef std::list<Atom> NegativeBody;
typedef boost::shared_ptr<Head> HeadPtr;
typedef boost::shared_ptr<PositiveBody> PositiveBodyPtr;
typedef boost::shared_ptr<NegativeBody> NegativeBodyPtr;

typedef std::pair<PositiveBodyPtr, NegativeBodyPtr> Body;

typedef std::pair<HeadPtr, Body> Rule;
typedef boost::shared_ptr<Rule> RulePtr;
typedef std::list<RulePtr> Rules;
typedef boost::shared_ptr<Rules> RulesPtr;

typedef std::pair<ContextTerm, Atom> BridgeAtom;
typedef boost::shared_ptr<BridgeAtom> BridgeAtomPtr;
typedef std::list<BridgeAtom> PositiveBridgeBody;
typedef std::list<BridgeAtom> NegativeBridgeBody;
typedef boost::shared_ptr<PositiveBridgeBody> PositiveBridgeBodyPtr;
typedef boost::shared_ptr<NegativeBridgeBody> NegativeBridgeBodyPtr;

typedef std::pair<PositiveBridgeBodyPtr, NegativeBridgeBodyPtr> BridgeBody;

typedef std::pair<HeadPtr, BridgeBody> BridgeRule;
typedef boost::shared_ptr<BridgeRule> BridgeRulePtr;
typedef std::list<BridgeRulePtr> BridgeRules;
typedef boost::shared_ptr<BridgeRules> BridgeRulesPtr;
typedef std::set<BridgeAtom> BridgeAtomSet;
typedef boost::shared_ptr<BridgeAtomSet> BridgeAtomSetPtr;

typedef std::vector<BridgeAtom> BridgeAtomVec;
typedef boost::shared_ptr<BridgeAtomVec> BridgeAtomVecPtr;
typedef BridgeAtomVec ReducedBridgeBody;
typedef BridgeAtomVecPtr ReducedBridgeBodyPtr;
typedef std::vector<ReducedBridgeBodyPtr> ReducedBridgeBodyVec;
typedef boost::shared_ptr<ReducedBridgeBodyVec> ReducedBridgeBodyVecPtr;

typedef std::list<ReducedBridgeBody::iterator> ReducedBridgeBodyIteratorList;
typedef boost::shared_ptr<ReducedBridgeBodyIteratorList> ReducedBridgeBodyIteratorListPtr;
typedef std::vector<ReducedBridgeBodyIteratorListPtr> ReducedBridgeBodyIteratorListVec;
typedef boost::shared_ptr<ReducedBridgeBodyIteratorListVec> ReducedBridgeBodyIteratorListVecPtr;


inline const Head&
getHead(const RulePtr& r)
{
  return *(r->first);
}


inline const Body&
getBody(const RulePtr& r)
{
  return r->second;
}


inline const PositiveBody&
getPositiveBody(const RulePtr& r)
{
  return *(r->second.first);
}


inline const NegativeBody&
getNegativeBody(const RulePtr& r)
{
  return *(r->second.second);
}


inline const Head&
getHead(const BridgeRulePtr& r)
{
  return *(r->first);
}


inline const BridgeBody&
getBody(const BridgeRulePtr& r)
{
  return r->second;
}


inline PositiveBridgeBody&
getPositiveBody(BridgeBody& B)
{
  return *(B.first);
}


inline const PositiveBridgeBody&
getPositiveBody(const BridgeBody& B) 
{
  return *(B.first);
}


inline const PositiveBridgeBody&
getPositiveBody(const BridgeRulePtr& r)
{
  return *(r->second.first);
}


inline PositiveBridgeBody&
getPositiveBody(BridgeRulePtr& r)
{
  return *(r->second.first);
}


inline NegativeBridgeBody&
getNegativeBody(BridgeBody& B)
{
  return *(B.second);
}


inline const NegativeBridgeBody&
getNegativeBody(const BridgeBody& B)
{
  return *(B.second);
}


inline const NegativeBridgeBody&
getNegativeBody(const BridgeRulePtr& r)
{
  return *(r->second.second);
}


inline NegativeBridgeBody&
getNegativeBody(BridgeRulePtr& r)
{
  return *(r->second.second);
}


inline std::ostream&
operator<< (std::ostream& os, const BridgeAtom& ba)
{
  return os << "(" << ctx2string(ba.first) << ":" << sb2string(ba.second) << ")";
}


inline std::ostream&
operator<< (std::ostream& os, const BridgeBody& B)
{
  const PositiveBridgeBody& pbody = getPositiveBody(B);
  PositiveBridgeBody::const_iterator p_end = --pbody.end();

  if (!pbody.empty())
    {
      if (pbody.size() > 1)
	{
	  //std::copy(pbody.begin(), p_end, std::ostream_iterator<BridgeAtom>(os, ", "));
	  for (PositiveBridgeBody::const_iterator p_it = pbody.begin(); p_it != p_end; ++p_it)
	    {
	      os << *p_it << ", ";
	    }
	}

      os << *p_end;
    }

  const NegativeBridgeBody& nbody = getNegativeBody(B);
  NegativeBridgeBody::const_iterator n_end = --nbody.end();

  if (!nbody.empty())
    {
      os << ", not ";
      
      if (nbody.size() > 1)
	{
	  //std::copy(nbody.begin(), n_end, std::ostream_iterator<BridgeAtom>(os, ", not "));
	  for (NegativeBridgeBody::const_iterator n_it = nbody.begin(); n_it != n_end; ++n_it)
	    {
	      os << *n_it << ", not ";
	    }
	}
      os << *n_end;
    }

  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const BridgeRulePtr& r)
{
  const Head& head = getHead(r);
  Head::const_iterator h_end = --head.end();

  if (!head.empty())
    {
      if (head.size() > 1)
	{
	  std::copy(head.begin(), h_end, std::ostream_iterator<Atom>(os, " v "));
	}
      os << *h_end;
    }

  os << ":- " << getBody(r);

  return os;
}


inline std::ostream&
operator<< (std::ostream& os, const BridgeRulesPtr& R)
{
  for (BridgeRules::const_iterator it = R->begin(); it != R->end(); ++it)
    {
      os << *it << "\n";
    }

  return os;
}

} // namespace dmcs

#endif /* RULE_H */

// Local Variables:
// mode: C++
// End:
