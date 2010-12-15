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
 * @file   Match.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Apr 22 22:34:40 2010
 * 
 * @brief  Match accessing and storing.
 * 
 * 
 */

#ifndef MATCH_H
#define MATCH_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/multi_index/composite_key.hpp>


#include <string>
#include <sstream>
#include <iosfwd>

namespace dmcs {

struct Match
{
  std::size_t srcCtx;  /// source context
  std::size_t sym;     /// symbol from source context
  std::size_t tarCtx;  /// target context
  std::size_t img;     /// image in the target context
  float       quality; /// match at some quality \in [0,1]

  Match(std::size_t src, std::size_t s, std::size_t tar, std::size_t i, float q)
    : srcCtx(src), sym(s), tarCtx(tar), img(i), quality(q)
  { }
};


// output of a Match
inline std::ostream&
operator<< (std::ostream& os, const Match& m)
{
  return os << '(' << m.srcCtx << ' ' << m.sym << ' ' << m.tarCtx << ' ' << m.img << ' ' << m.quality << ')';
}



/// tags used to access Match columns by name
namespace Tag
{
  struct SrcSym       {};
  struct SrcSymTar    {};
  struct SrcSymTarImg {};
  struct SrcCtx       {};
  struct TarCtx       {};
  struct Image        {};
  struct Quality      {};
}


/// a MatchTable is a table with 5 columns named SrcCtx, Sym, TarCtx, Image, Quality
/// in which the first 4 columns form a key
typedef boost::multi_index_container<
  Match,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<
      boost::multi_index::tag<Tag::SrcSymTarImg>,
      boost::multi_index::composite_key<
	Match,
	boost::multi_index::member<Match, std::size_t, &Match::srcCtx>,
	boost::multi_index::member<Match, std::size_t, &Match::sym>,
	boost::multi_index::member<Match, std::size_t, &Match::tarCtx>,
	boost::multi_index::member<Match, std::size_t, &Match::img>
	>
      >,

    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Tag::SrcSym>,
      boost::multi_index::composite_key<
	Match,
	boost::multi_index::member<Match, std::size_t, &Match::srcCtx>,
	boost::multi_index::member<Match, std::size_t, &Match::sym>
	>
      >,

    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Tag::SrcSymTar>,
      boost::multi_index::composite_key<
	Match,
	boost::multi_index::member<Match, std::size_t, &Match::srcCtx>,
	boost::multi_index::member<Match, std::size_t, &Match::sym>,
	boost::multi_index::member<Match, std::size_t, &Match::tarCtx>
	>
      >,

    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Tag::SrcCtx>,
	boost::multi_index::member<Match, std::size_t, &Match::srcCtx>
      >
    >
  > MatchTable;


typedef boost::shared_ptr<MatchTable> MatchTablePtr;

/// query MatchTable by source context 
typedef boost::multi_index::index<MatchTable, Tag::SrcCtx>::type MatchTableBySrcCtx;

/// query MatchTable by source context and source belief
typedef boost::multi_index::index<MatchTable, Tag::SrcSym>::type MatchTableBySrcSym;

/// query MatchTable by source context, source belief, and target context
typedef boost::multi_index::index<MatchTable, Tag::SrcSymTar>::type MatchTableBySrcSymTar;

/// query MatchTable by source context, source belief, target context, image
typedef boost::multi_index::index<MatchTable, Tag::SrcSymTarImg>::type MatchTableBySrcSymTarImg;

typedef std::vector<MatchTableBySrcSym::iterator> MatchTableIteratorVec;
typedef boost::shared_ptr<MatchTableIteratorVec> MatchTableIteratorVecPtr;

typedef std::list<MatchTableIteratorVec::iterator> MatchTableIteratorVecIteratorList;
typedef boost::shared_ptr<MatchTableIteratorVecIteratorList> MatchTableIteratorVecIteratorListPtr;

class CompareMatch
{
public:
  CompareMatch(std::size_t ctx_id_)
    : ctx_id(ctx_id_)
  { }

  bool
  operator()(const MatchTableBySrcSym::iterator it)
  {
    return (it->tarCtx == ctx_id);
  }

private:
  std::size_t ctx_id;
};

// output the whole MatchTable
inline std::ostream&
operator<< (std::ostream& os, const MatchTable& mt)
{
  if (!mt.empty())
    {
      MatchTable::const_iterator end = --mt.end();

      if (mt.size() > 1)
	{
	  std::copy(mt.begin(), end, std::ostream_iterator<Match>(os, "\n"));
	}

      if (mt.size() > 0)
	{
	  os << *end;
	}
    }

  return os;
}



// read from stream and update Signature
inline std::istream&
operator>> (std::istream& is, MatchTable& mt)
{
  std::string s;

  // watch out, we can only read one line exaclty, otw. read_graphviz
  // is getting confused and kaputt
  std::getline(is, s);

  typedef boost::escaped_list_separator<char> StringSeparator;

  StringSeparator ssep("\\", ",", "()");
  boost::tokenizer<StringSeparator> tok(s, ssep);

  StringSeparator esep("()", " ", "");
  
  for (boost::tokenizer<StringSeparator>::iterator it = tok.begin(); it != tok.end(); ++it)
    {
      std::string trimmed = *it;

      boost::trim(trimmed);
      
      boost::tokenizer<StringSeparator> stok(trimmed, esep);
      
      boost::tokenizer<StringSeparator>::iterator sit = stok.begin();

      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("Got no symbol list");
	}
      
      std::size_t src_ctx = std::atoi(sit->c_str());
      ++sit;
      
      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 1");
	}
      
      std::size_t sym = std::atoi(sit->c_str());
      ++sit;

      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 2");
	}
      
      std::size_t tar_ctx = std::atoi(sit->c_str());
      ++sit;
      
      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 3");
	}
      

      std::size_t img = std::atoi(sit->c_str());
      ++sit;

      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 4");
	}

      std::istringstream iss(*sit);
      float qual;
      if (!(iss >> qual))
	{
	  throw boost::escaped_list_error("quality not a float");
	}

      mt.insert(Match(src_ctx, sym, tar_ctx, img, qual));
    }

  return is;
}


// need to find a proper location for this type

typedef std::vector<std::size_t> CountVec;
typedef boost::shared_ptr<CountVec> CountVecPtr;


// used to calculate the number of edges in the intial topology
inline
std::size_t
no_unique_connections(MatchTablePtr mt, std::size_t poolsize)
{
  std::size_t nuc = 0;
  bool mark[poolsize][poolsize];

  for (std::size_t i = 0; i < poolsize; ++i)
    {
      for (std::size_t j = 0; j < poolsize; ++j)
	{
	  mark[i][j] = false;
	}
    }

  const MatchTableBySrcCtx& sc = boost::get<Tag::SrcCtx>(*mt);

  for (MatchTableBySrcCtx::const_iterator it = sc.begin(); it != sc.end(); ++it)
    {
      ///@todo: std::size_t ~~> ContextID
      std::size_t s = it->srcCtx;
      std::size_t t = it->tarCtx;

      if (!mark[s-1][t-1])
	{
	  nuc++;
	  mark[s-1][t-1] = true;
	}
    }

  return nuc;
}

} // namespace dmcs

#endif // MATCH_H

// Local Variables:
// mode: C++
// End:
