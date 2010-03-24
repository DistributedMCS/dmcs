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
 * @file   Signature.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sat Jan 16 18:38:40 2010
 * 
 * @brief  Signature accessing and storing.
 * 
 * 
 */

#if !defined(_SIGNATURE_H)
#define _SIGNATURE_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <string>
#include <iosfwd>

namespace dmcs {

/// a Symbol is a quadruple composed of a
/// - symbol string;
/// - context id i from some C_i;
/// - unique local id from a contiguous id space (used as id for a local solver); and
/// - unique id in a context C_i (used as offset in a belief state).
struct Symbol
{
  std::string sym;     /// symbol string
  std::size_t ctxId;   /// symbol belongs to context ctxId
  std::size_t localId; /// symbol has locally a unique ID from a contiguous ID space
  std::size_t origId;  /// symbol has a unique ID in context ctxId

  Symbol(std::string s, std::size_t i, std::size_t l, std::size_t o)
    : sym(s), ctxId(i), localId(l), origId(o)
  { }
};


// output of a Symbol tuple
inline std::ostream&
operator<< (std::ostream& os, const Symbol& s)
{
  return os << '(' << s.sym << ' ' << s.ctxId << ' ' << s.localId << ' ' << s.origId << ')';
}



/// tags used to access Signature columns by name
namespace Tag
{
  struct Sym {};
  struct Ctx {};
  struct Local {};
}


/// a Signature is a table with 3 columns named Sym, Ctx, and Local,
/// where Sym and Local are unique, and Ctx is non-unique
typedef boost::multi_index_container<
  Symbol,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<
      boost::multi_index::tag<Tag::Sym>,
      boost::multi_index::member<Symbol, std::string, &Symbol::sym>
      >,
    boost::multi_index::ordered_non_unique<
      boost::multi_index::tag<Tag::Ctx>,
      boost::multi_index::member<Symbol, std::size_t, &Symbol::ctxId>
      >,
    boost::multi_index::ordered_unique<
      boost::multi_index::tag<Tag::Local>,
      boost::multi_index::member<Symbol, std::size_t, &Symbol::localId>
      >
    >
  > Signature;


typedef boost::shared_ptr<Signature> SignaturePtr;

// output the whole Signature
inline std::ostream&
operator<< (std::ostream& os, const Signature& sig)
{
  if (!sig.empty())
    {
      Signature::const_iterator end = --sig.end();

      if (sig.size() > 1)
	{
	  std::copy(sig.begin(), end, std::ostream_iterator<Symbol>(os, ","));
	}

      if (sig.size() > 0)
	{
	  os << *end;
	}
    }

  return os;
}


// read from stream and update Signature
inline std::istream&
operator>> (std::istream& is, Signature& sig)
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
      
      std::string sym = *sit;
      ++sit;
      
      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 1");
	}
      
      std::size_t ctx = std::atoi(sit->c_str());
      ++sit;
      
      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 2");
	}
      
      std::size_t local = std::atoi(sit->c_str());
      ++sit;
      
      if (sit == stok.end())
	{
	  throw boost::escaped_list_error("symbol list length == 3");
	}
      
      std::size_t orig = std::atoi(sit->c_str());
      
      sig.insert(Symbol(sym, ctx, local, orig));
    }

  return is;
}


/// query Signature by symbol string
typedef boost::multi_index::index<Signature,Tag::Sym>::type SignatureBySym;

/// query Signature by context ID
typedef boost::multi_index::index<Signature,Tag::Ctx>::type SignatureByCtx;

/// query Signature by local ID
typedef boost::multi_index::index<Signature,Tag::Local>::type SignatureByLocal;

} // namespace dmcs

#endif // _SIGNATURE_H

// Local Variables:
// mode: C++
// End:
