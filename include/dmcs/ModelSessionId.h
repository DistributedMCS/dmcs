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
 * @file   ModelSessionId.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Mar  11 15:23:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef MODEL_SESSION_ID_H
#define MODEL_SESSION_ID_H

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/list.hpp>

#include "dmcs/Log.h"

namespace dmcs {



struct ModelSessionId
{
  PartialBeliefState* partial_belief_state;
  std::size_t path;
  std::size_t session_id;

  ModelSessionId()
  { }

  ModelSessionId(PartialBeliefState* p, 
		 std::size_t pa,
		 std::size_t sid)
    : partial_belief_state(p), 
      path(pa),
      session_id(sid)
  { }


  /*  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version *//*)
{
    ar & partial_belief_state;
    ar & path;
    ar & session_id;

    delete partial_belief_state;
    partial_belief_state = 0;
  }*/
};



typedef std::list<ModelSessionId> ModelSessionIdList;
typedef boost::shared_ptr<ModelSessionIdList> ModelSessionIdListPtr;



inline std::ostream&
operator<< (std::ostream& os, const ModelSessionId& ms)
{
  if (ms.partial_belief_state)
    {
      os << "(" << *(ms.partial_belief_state) << ", " << ms.session_id << ")";
    }
  else
    {
      os << "(NULL, " << ms.session_id << ")";
    }


  return os;
}



inline std::ostream&
operator<< (std::ostream& os, const ModelSessionIdList& msl)
{
  os << "{ " ;
  for (ModelSessionIdList::const_iterator it = msl.begin(); it != msl.end(); ++it)
    {
      os << *it << std::endl;
    }
  os << " }";

  return os;
}



inline bool
my_compare(const ModelSessionId& ms1, const ModelSessionId& ms2)
{
  assert (ms1.partial_belief_state && ms2.partial_belief_state);

  if (*(ms1.partial_belief_state) < *(ms2.partial_belief_state))
    {
      return true;
    }
  else if (*(ms1.partial_belief_state) == *(ms2.partial_belief_state))
    {
      if (ms1.session_id > ms2.session_id)
	{
	  return true;
	}
    }
  
  return false;
}



inline bool
operator== (const ModelSessionId& ms1, const ModelSessionId& ms2)
{
  assert (ms1.partial_belief_state && ms2.partial_belief_state);

  return (*(ms1.partial_belief_state) == *(ms2.partial_belief_state));
}



} // namespace dmcs




namespace boost {

namespace serialization {

template<class Archive>
inline void save(Archive& ar, const dmcs::ModelSessionId& msi, const unsigned int file_version)
{
  ar & msi.partial_belief_state;
  ar & msi.path;
  ar & msi.session_id;

  
}



template<class Archive>
inline void load(Archive& ar, dmcs::ModelSessionId& msi, const unsigned int file_version)
{
  ar & msi.partial_belief_state;
  ar & msi.path;
  ar & msi.session_id;

  DMCS_LOG_TRACE("Loading. pointer = " << msi.partial_belief_state);

  /*DMCS_LOG_TRACE("Going to delete partial_belief_state. pointer = " << msi.partial_belief_state);

  delete msi.partial_belief_state;

  DMCS_LOG_TRACE("Done with deleting partial_belief_state");*/
}



template<class Archive>
inline void serialize(Archive& ar, dmcs::ModelSessionId& msi, const unsigned int file_version)
{
  split_free(ar, msi, file_version);
}



} // namespace serialization

} // namespace boost

#endif // MODEL_SESSION_ID_H

// Local Variables:
// mode: C++
// End:
