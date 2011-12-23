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
 * @file   BridgeRuleTable.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  14 10:02:20 2011
 * 
 * @brief  Table for storing BridgeRules.
 * 
 * 
 */

#ifndef BRIDGE_RULE_TABLE_H
#define BRIDGE_RULE_TABLE_H

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/shared_ptr.hpp>

#include "mcs/BridgeRule.h"
#include "mcs/Table.h"

namespace dmcs {

class BridgeRuleTable :
    public Table<
  BridgeRule,
  // index is
  boost::multi_index::indexed_by<
    // address = running ID for constant access
    boost::multi_index::random_access<
      boost::multi_index::tag<impl::AddressTag>
      >,
	// kind
	boost::multi_index::ordered_non_unique<
	  boost::multi_index::tag<impl::KindTag>,
	  BOOST_MULTI_INDEX_MEMBER(BridgeRule, IDKind, kind)
	  >,
    // element
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<impl::ElementTag>,
      boost::multi_index::composite_key<
	BridgeRule,
	BOOST_MULTI_INDEX_MEMBER(BridgeRule, IDKind, kind),
	BOOST_MULTI_INDEX_MEMBER(BridgeRule, ID,     head),
	BOOST_MULTI_INDEX_MEMBER(BridgeRule, Tuple,  body)
	>
      >
    >
  >
{
public:
  typedef Container::index<impl::AddressTag>::type AddressIndex;
  typedef Container::index<impl::KindTag>::type KindIndex;
  typedef Container::index<impl::ElementTag>::type ElementIndex;
  typedef ElementIndex::iterator ElementIterator;
	// methods
public:
  // retrieve by ID
  // assert that id.kind is correct for BridgeRule
  // assert that ID exists in table
  inline const BridgeRule& getByID(ID id) const throw ();

  // get the ID of the rule
  inline ID getIDByElement(const BridgeRule& rule) const throw();
  
  // store rule (no duplicate check is done/required)
  inline ID storeAndGetID(const BridgeRule& rule) throw();
  inline void clear();
  
  // update
  // (oldStorage must be from getByID() or from *const_iterator)
  inline void update(const BridgeRule& oldStorage, BridgeRule& newStorage) throw();
};

typedef boost::shared_ptr<BridgeRuleTable> BridgeRuleTablePtr;


// retrieve by ID
// assert that id.kind is correct for Rule
// assert that ID exists
const BridgeRule&
BridgeRuleTable::getByID(ID id) const throw ()
{
  assert(id.isBridgeRule());
  ReadLock lock(mutex);
  const AddressIndex& idx = container.get<impl::AddressTag>();
  // the following check only works for random access indices, but here it is ok
  assert( id.address < idx.size() );
  return idx.at(id.address);
}


// getID by rule
ID 
BridgeRuleTable::getIDByElement(const BridgeRule& rule) const throw()
{
  ReadLock lock(mutex);
  const ElementIndex& sidx = container.get<impl::ElementTag>();
  ElementIndex::const_iterator it = sidx.find( boost::make_tuple(rule.kind, rule.head, rule.body) );
  if( it == sidx.end() )
    {
      return ID_FAIL;
    }
  else
    {
      const AddressIndex& aidx = container.get<impl::AddressTag>();
      return ID(it->kind, // kind
		container.project<impl::AddressTag>(it) - aidx.begin() // address
		);
    }
}


// store rule (no duplicate check is done/required)
ID 
BridgeRuleTable::storeAndGetID(const BridgeRule& rule) throw()
{
  assert(ID(rule.kind,0).isBridgeRule());
  
  assert(!rule.body.empty());

  AddressIndex::const_iterator it;
  bool success;

  WriteLock lock(mutex);
  AddressIndex& idx = container.get<impl::AddressTag>();
  boost::tie(it, success) = idx.push_back(rule);

  (void)success;
  assert(success);

  return ID(
	    rule.kind, // kind
	    container.project<impl::AddressTag>(it) - idx.begin() // address
	    );
}


void 
BridgeRuleTable::clear()
{
  WriteLock lock(mutex);
  container.clear();
}

} // namespace dmcs

#endif // BRIDGE_RULE_TABLE_H

// Local Variables:
// mode: C++
// End:
