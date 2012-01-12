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
 * @file   BeliefTable.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  12 11:22:24 2011
 * 
 * @brief  Table for storing Beliefs
 */

#ifndef BELIEF_TABLE_H
#define BELIEF_TABLE_H

#include "mcs/Belief.h"
#include "mcs/Table.h"

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

namespace dmcs {

class BeliefTable :
    public Table<
  // value type is Belief struct
  Belief,
  // index is
  boost::multi_index::indexed_by<
    // address = running ID for constant access
    boost::multi_index::random_access<
      boost::multi_index::tag<impl::AddressTag>
      >,
    // unique IDs for unique symbol strings
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<impl::BeliefTag>,
      BOOST_MULTI_INDEX_MEMBER(Belief, std::string, text)
      >
    >
  >
{
  
// types
public:
  typedef Container::index<impl::AddressTag>::type AddressIndex;
  typedef Container::index<impl::BeliefTag>::type BeliefIndex;

// methods
public:
  // retrieve by ID
  // assert that id.kind is correct for Term
  // assert that ID exists
  inline const Belief& getByID(ID id) const throw ();
  
  // given string, look if already stored
  // if no, return ID_FAIL, otherwise return ID
  inline ID getIDByString(const std::string& str) const throw();
  
  // store text, assuming it does not exist
  // assert that text did not exist
  inline ID storeAndGetID(const Belief& symb) throw();
  
  // retrieve range by kind (return lower/upper bound iterators, +provide method to get ID from iterator)
};

typedef boost::shared_ptr<BeliefTable> BeliefTablePtr;

// retrieve by ID
// assert that id.kind is correct for Term
// assert that ID exists
const Belief&
BeliefTable::getByID(ID id) const throw ()
{
  assert(id.isBelief());

  ReadLock lock(mutex);
  const AddressIndex& idx = container.get<impl::AddressTag>();
  // the following check only works for random access indices, but here it is ok
  assert( id.address < idx.size() );
  return idx.at(id.address);
}

// given string, look if already stored
// if no, return ID_FAIL, otherwise return ID
ID BeliefTable::getIDByString(const std::string& str) const throw()
{
  //typedef Container::index<impl::BeliefTag>::type BeliefIndex;
  ReadLock lock(mutex);
  const BeliefIndex& sidx = container.get<impl::BeliefTag>();
  BeliefIndex::const_iterator it = sidx.find(str);
  if( it == sidx.end() )
    return ID_FAIL;
  else
    {
      const AddressIndex& aidx = container.get<impl::AddressTag>();
      return ID(
		it->kind, // kind
		container.project<impl::AddressTag>(it) - aidx.begin() // address
		);
    }
}

// store text, assuming it does not exist
// assert that text did not exist
ID BeliefTable::storeAndGetID(const Belief& b) throw()
{
  assert(ID(b.kind,0).isBelief());
  assert(!b.text.empty());
  
  bool success;
  AddressIndex::const_iterator it;

  WriteLock lock(mutex);
  AddressIndex& idx = container.get<impl::AddressTag>();
  boost::tie(it, success) = idx.push_back(b);
  (void)success;
  assert(success);
  
  return ID(b.kind, // kind
	    container.project<impl::AddressTag>(it) - idx.begin() // address
	    );
}


} // namespace dmcs

#endif // BELIEF_TABLE_H

// Local Variables:
// mode: C++
// End:
