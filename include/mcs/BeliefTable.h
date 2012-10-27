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

#include <vector>

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcs {

class BeliefUnknownException:
  public std::runtime_error
{
  public:
    BeliefUnknownException(IDAddress addr):
      std::runtime_error("belief unknown during BeliefTable lookup!"),
      addr(addr) {}

  private:
    IDAddress addr;
};

class BeliefTable :
    public Table<
  // value type is Belief struct
  Belief,
  // index is
  boost::multi_index::indexed_by<
    // no random_access, as we might need very sparse and incomplete belief tables for all other contexts, IDs are determined by query plan!
    // address = running ID for constant access
    //boost::multi_index::random_access<
    //  boost::multi_index::tag<impl::AddressTag>
    //  >,
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<impl::AddressTag>,
      BOOST_MULTI_INDEX_MEMBER(Belief, IDAddress, address)
    >,
    // unique IDs for unique symbol strings
    boost::multi_index::hashed_unique<
      boost::multi_index::tag<impl::BeliefTag>,
      BOOST_MULTI_INDEX_MEMBER(Belief, std::string, text)
      >
    >
  >,
  private ostream_printable<BeliefTable>
{
  
// types
public:
  typedef Container::index<impl::AddressTag>::type AddressIndex;
  typedef Container::index<impl::BeliefTag>::type BeliefIndex;

// methods
public:
  BeliefTable():
    nextAddress(0) {}
  virtual ~BeliefTable();

  // retrieve by ID
  // assert that id.kind is correct for Term
  // assert that ID exists
  inline const Belief& getByID(ID id) const throw (BeliefUnknownException);
  
  // given string, look if already stored
  // if no, return ID_FAIL, otherwise return ID
  inline ID getIDByString(const std::string& str) const throw();
  
  // store text, assuming this belief is not yet stored
  inline ID storeAndGetID(const Belief& symb) throw();

  // store whole belief, assuming this belief is not yet stored
  inline ID storeWithID(const Belief& symb, ID setid) throw();

  virtual std::ostream& print(std::ostream& o) const;
  
  //TODO: find a nicer way to print the BeliefTable for generator purpose
  std::string
  gen_print() const;
protected:
  IDAddress nextAddress;
};

typedef boost::shared_ptr<BeliefTable> BeliefTablePtr;

// retrieve by ID
// assert that id.kind is correct for Term
// assert that ID exists
const Belief&
BeliefTable::getByID(ID id) const throw (BeliefUnknownException)
{
  assert(id.isBelief());

  ReadLock lock(mutex);
  const AddressIndex& idx = container.get<impl::AddressTag>();
  AddressIndex::const_iterator it = idx.find(id.address);
  if( it == idx.end() )
    throw BeliefUnknownException(id.address);
  else
    return *it;
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
    return ID(it->kind, it->address);
}

// store text, assuming this belief is not yet stored
ID BeliefTable::storeAndGetID(const Belief& b) throw()
{
  assert(ID(b.kind,0).isBelief());
  assert(!b.text.empty());
  
  WriteLock lock(mutex);
  // copy belief to new belief
  Belief nb(b);
  // set address
  nb.address = nextAddress;
  // increment for next new address
  nextAddress++;

  // store into idx
  AddressIndex& idx = container.get<impl::AddressTag>();
  bool success;
  AddressIndex::const_iterator it;
  boost::tie(it, success) = idx.insert(nb);
  (void)success;
  assert(success);
  
  return ID(nb.kind, nb.address);
}

// store whole belief, assuming this belief is not yet stored
ID BeliefTable::storeWithID(const Belief& b, ID setid) throw()
{
  assert(setid.isBelief());
  assert(!b.text.empty());
  assert(nextAddress == 0 && "if you use storeWithID you should not also use storeAndGetID");
  
  // copy belief to new belief
  Belief nb(b);
  // set kind and address
  nb.kind = setid.kind;
  nb.address = setid.address;

  WriteLock lock(mutex);

  // store into idx
  bool success;
  AddressIndex::const_iterator it;
  AddressIndex& idx = container.get<impl::AddressTag>();
  boost::tie(it, success) = idx.insert(nb);
  (void)success;
  assert(success);
  
  return setid;
}

typedef boost::shared_ptr<BeliefTable> BeliefTablePtr;
typedef std::vector<BeliefTablePtr> BeliefTableVec;
typedef boost::shared_ptr<BeliefTableVec> BeliefTableVecPtr;

} // namespace dmcs

#endif // BELIEF_TABLE_H

// Local Variables:
// mode: C++
// End:
