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
 * @file   Printhelpers.h
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Storage classes for atoms: Atom, OrdinaryAtom, BuiltinAtom, AggregateAtom, BridgeAtom
 */

#ifndef PRINTHELPERS_H
#define PRINTHELPERS_H

#include <boost/range/iterator_range.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <list>

// with this class, you can make your own classes ostream-printable
//
// usage:
//   derive YourType from ostream_printable<YourType>
//   implement std::ostream& YourType::print(std::ostream& o) const;
//   now you can << YourType << and it will use the print() function
// see http://en.wikipedia.org/wiki/Barton–Nackman_trick
template<typename T>
class ostream_printable
{
  friend std::ostream& operator<<(std::ostream& o, const T& t)
  { 
    return t.print(o); 
  }

  // to be defined in derived class:
  //std::ostream& print(std::ostream& o) const;
};

// if some class has a method "std::ostream& print(std::ostream&) const"
// and you have an object o of this type
// then you can do "std::cerr << ... << print_method(o) << ... " to print it

// if some other method is used to print T foo
// e.g. std::ostream& BAR::printFOO(std::ostream& o, const FOO& p) const
// then you can do 
// std::cerr << ... << print_function(boost::bind(&BAR::printFOO, &bar, _1, foo)) << ...
// e.g. std::ostream& printFOO(std::ostream& o, const FOO& p) const
// then you can do 
// std::cerr << ... << print_function(boost::bind(&printFoo, _1, foo)) << ...

// std::cerr << ... << printopt(boost::optional<T>) << ...
// gives "unset" or prints T's contents

// std::cerr << ... << printptr(T* or boost::shared_ptr<T>) << ...
// gives "null" or prints T* as a void*

// std::cerr << ... << printrange(Range r) << ... prints generic ranges enclosed in "<>"
// std::cerr << ... << printvector(std::vector<T>) << ... prints generic vectors enclosed in "[]"
// std::cerr << ... << printset(std::set<T>) << ... prints generic sets enclosed in "{}"

struct print_container
{
  virtual ~print_container() {}
  virtual std::ostream& print(std::ostream& o) const = 0;
};

inline std::ostream& operator<<(std::ostream& o, print_container* c)
{
  assert(c);
  std::ostream& ret = c->print(o);
  delete c;
  return ret;
}

template<typename T>
struct print_stream_container : public print_container
{
  T t;
  print_stream_container(const T& t): t(t) {}
  virtual ~print_stream_container() {}

  virtual std::ostream& print(std::ostream& o) const
  { 
    return o << t; 
  }
};

struct print_method_container : public print_container
{
  typedef boost::function<std::ostream& (std::ostream&)> PrintFn;

  PrintFn fn;
  print_method_container(const PrintFn& fn): fn(fn) {}
  virtual ~print_method_container() {}

  virtual std::ostream& print(std::ostream& o) const
  { 
    return fn(o); 
  }
};

// this can be used if T contains a method "ostream& print(ostream&) const"
template<typename T>
inline print_container* print_method(const T& t)
{
  return new print_method_container(boost::bind(&T::print, &t, _1));
}

// this can be used if some third party method is used to print T
// e.g. std::ostream& BAR::printFOO(std::ostream& o, const FOO& p) const
// is printed as
// ... << print_function(boost::bind(&BAR::printFOO, &bar, _1, foo)) << ...
inline print_container* print_function(const print_method_container::PrintFn& fn)
{
  return new print_method_container(fn);
}

template<typename T>
inline print_container* printopt(const boost::optional<T>& t)
{
  if( !!t )
    return new print_stream_container<const T&>(t.get());
  else
    return new print_stream_container<const char*>("unset");
}

template<typename T>
inline print_container* printptr(const boost::shared_ptr<T>& t)
{
  if( !!t )
    return new print_stream_container<const void*>(
        reinterpret_cast<const void*>(t.get()));
  else
    return new print_stream_container<const char*>("null");
}

template<typename T>
inline print_container* printptr(const boost::shared_ptr<const T>& t)
{
  if( t != 0 )
    return new print_stream_container<const void*>(
        reinterpret_cast<const void*>(t.get()));
  else
    return new print_stream_container<const char*>("null");
}

template<typename T>
inline print_container* printptr(const T* const t)
{
  if( t != 0 )
    return new print_stream_container<const void*>(
        reinterpret_cast<const void* const>(t));
  else
    return new print_stream_container<const char*>("null");
}

template<typename Range>
inline print_container* printrange(Range r,
				   const char* open="<",
				   const char* sep=",", 
				   const char* close=">")
{
  std::ostringstream o;
  o << open;
  typename Range::const_iterator it = boost::begin(r);
  typename Range::const_iterator itend = boost::end(r);
  if( it != itend )
  {
    o << *it;
    it++;
  }
  for(; it != itend; ++it)
    o << sep << *it;
  o << close;
  return new print_stream_container<std::string>(o.str());
}

template<typename T>
inline print_container* printset(const std::set<T>& t,
				 const char* open="{", 
				 const char* sep=",", 
				 const char* close="}")
{
  return printrange(t, open, sep, close);
}

template<typename T>
inline print_container* printvector(const std::vector<T>& t,
				    const char* open="[", 
				    const char* sep=",", 
				    const char* close="]")
{
  return printrange(t, open, sep, close);
}

template<typename T>
inline print_container* printlist(const std::list<T>& t,
				  const char* open="(", 
				  const char* sep=",", 
				  const char* close=")")
{
  return printrange(t, open, sep, close);
}


#endif // PRINTHELPERS_H

// Local Variables:
// mode: C++
// End:
