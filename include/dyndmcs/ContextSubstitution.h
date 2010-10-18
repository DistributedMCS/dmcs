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
 * @file   ContextSubstitution.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Apr  27 09:59:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef CONTEXT_SUBSTITUTION_H
#define CONTEXT_SUBSTITUTION_H

#include <list>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <iostream>

#include "Variable.h"
#include "Match.h"

namespace dmcs {

///@todo: currently we have to copy to make new ContextSubstitution(s)
/// in DynamicConfiguration.bind_rule(). A more efficient data
/// structure can circumvent this copying, e.g., use graph for
/// ContextSubstitution.

typedef std::size_t ContextID;
typedef std::list<ContextID> ContextList;
typedef boost::shared_ptr<ContextList> ContextListPtr;
  //typedef std::pair<ContextTerm, ContextID> ContextMatch;
  //typedef std::map<ContextTerm,  ContextID> ContextSubstitution;


typedef std::vector<ContextID> ContextVec;
typedef boost::shared_ptr<ContextVec> ContextVecPtr;

typedef std::list<ContextVec::iterator> ContextVecIteratorList;
typedef boost::shared_ptr<ContextVecIteratorList> ContextVecIteratorListPtr;


// similar to Match, except that a context term representing the
// source instead of a context id
struct ContextMatch
{
  ContextMatch()
  { }
  
  ContextMatch(ContextTerm ctx_term_, std::size_t sym_,
	       ContextID tarCtx_, std::size_t img_, float quality_)
    : ctx_term(ctx_term_), sym(sym_), tarCtx(tarCtx_), 
      img(img_), quality(quality_)
  { }

  ContextTerm ctx_term;
  std::size_t sym;
  ContextID   tarCtx;
  std::size_t img;
  float       quality;

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */)
  {
    ar & ctx_term;
    ar & sym;
    ar & tarCtx;
    ar & img;
    ar & quality;
  }
};


inline
std::ostream&
operator<< (std::ostream& os, const ContextMatch& cm)
{
  return os << "(" << ctx2string(cm.ctx_term) << ", " << cm.sym << ") --> (" << cm.tarCtx << ", " << cm.img << ")";
}

typedef boost::multi_index_container<
  ContextMatch,
  boost::multi_index::indexed_by<

    boost::multi_index::ordered_unique<
      boost::multi_index::tag<Tag::SrcSymTarImg>,
      boost::multi_index::composite_key<
      ContextMatch,
      boost::multi_index::member<ContextMatch, ContextTerm, &ContextMatch::ctx_term>,
      boost::multi_index::member<ContextMatch, std::size_t, &ContextMatch::sym>,
      boost::multi_index::member<ContextMatch, ContextID,   &ContextMatch::tarCtx>,
      boost::multi_index::member<ContextMatch, std::size_t, &ContextMatch::img>
	>
      >,

    boost::multi_index::ordered_unique<
      boost::multi_index::tag<Tag::SrcSym>,
      boost::multi_index::composite_key<
      ContextMatch,
      boost::multi_index::member<ContextMatch, ContextTerm, &ContextMatch::ctx_term>,
      boost::multi_index::member<ContextMatch, std::size_t, &ContextMatch::sym>
	>
      >,    

    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Tag::TarCtx>,
      boost::multi_index::member<ContextMatch, ContextID, &ContextMatch::tarCtx>
      >

    >
  > ContextSubstitution;

typedef boost::multi_index::index<ContextSubstitution, Tag::SrcSymTarImg>::type ContextSubstitutionBySrcSymTarImg;
typedef boost::multi_index::index<ContextSubstitution, Tag::SrcSym>::type ContextSubstitutionBySrcSym;
typedef boost::multi_index::index<ContextSubstitution, Tag::TarCtx>::type ContextSubstitutionByTarCtx;

typedef boost::shared_ptr<ContextSubstitution> ContextSubstitutionPtr;
typedef std::list<ContextSubstitutionPtr> ContextSubstitutionList;
typedef boost::shared_ptr<ContextSubstitutionList> ContextSubstitutionListPtr;


/*inline std::ostream&
operator<< (std::ostream& os, const ContextSubstitution& cs)
 {
  os << "(";
  if (!cs.empty())
     {
      ContextSubstitution::const_iterator end = --cs.end();
      for (ContextSubstitution::const_iterator it = cs.begin(); it != end; ++it)
 	{
	  os << ctx2string(it->first) << " -> " << it->second << ", ";
 	}
      os << ctx2string(end->first) << " -> " << end->second;
     }
 
  return os << ")";
  }*/


inline std::ostream&
operator<< (std::ostream& os, const ContextSubstitution& cs)
{
  const ContextSubstitutionBySrcSymTarImg& cs_ssti = boost::get<Tag::SrcSymTarImg>(cs);

  os << "[";
  if (!cs_ssti.empty())
    {
      ContextSubstitutionBySrcSymTarImg::const_iterator end = --cs_ssti.end();
      for (ContextSubstitutionBySrcSymTarImg::const_iterator it = cs_ssti.begin(); it != end; ++it)
	{
	  os << *it << "; ";
	}
      os << *end;
    }

  return os << "]";
}


inline std::ostream&
operator<< (std::ostream& os, const ContextSubstitutionList& css)
{
  if (!css.empty())
    {
      ContextSubstitutionList::const_iterator end = --css.end();
      
      for (ContextSubstitutionList::const_iterator it = css.begin(); it != end; ++it)
	{
	  os << **it << " --- " << std::endl;
	}
      os << **end;
    }
  return os;
}


inline bool
already_chosen(const ContextSubstitutionPtr& cs, ContextID ck, ContextID c)
{
  // check whether there is already a match from a different context
  // than ck to c
  const ContextSubstitutionByTarCtx& cs_tar = boost::get<Tag::TarCtx>(*cs);

  ContextSubstitutionByTarCtx::const_iterator low = cs_tar.lower_bound(c);
  ContextSubstitutionByTarCtx::const_iterator up  = cs_tar.upper_bound(c);

  for (; low != up; ++low)
    {
      if (ctxID(low->ctx_term) != ck)
	{
	  return true;
	}
    }

  return false;
}


inline bool
already_chosen(const ContextSubstitutionPtr& cs, ContextID c)
{
  const ContextSubstitutionByTarCtx& cs_tar = boost::get<Tag::TarCtx>(*cs);

  ContextSubstitutionByTarCtx::const_iterator low = cs_tar.lower_bound(c);
  ContextSubstitutionByTarCtx::const_iterator up  = cs_tar.upper_bound(c);

  if (low != up)
    {
      return true;
    }

  return false;
}


inline
float
average_quality(const ContextSubstitutionPtr ctx_subs)
{
  float tmp = 0;

  for (ContextSubstitution::const_iterator it = ctx_subs->begin(); it != ctx_subs->end(); ++it)
    {
      tmp += it->quality;
    }

  tmp /= ctx_subs->size();

  return tmp;
}


} // namespace dmcs

#endif // CONTEXT_SUBSTITUTION_H

// Local Variables:
// mode: C++
// End:
