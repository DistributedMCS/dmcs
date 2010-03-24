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

#include <list>
#include <set>
#include <boost/shared_ptr.hpp>


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

typedef std::pair<std::size_t, Atom> BridgeAtom;
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


} // namespace dmcs

#endif /* RULE_H */

// Local Variables:
// mode: C++
// End:
