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
 * @file   ClaspResultGrammar.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:40:39 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef CLASP_RESULT_GRAMMAR_H
#define CLASP_RESULT_GRAMMAR_H

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_common.hpp>


namespace dmcs {


struct ClaspResultGrammar
  : public BOOST_SPIRIT_CLASSIC_NS :: grammar<ClaspResultGrammar>
{
  enum RuleTags
    {
      None = 0,
      Root,
      Neg,
      Variable,
      Sentinel,
      Literal,
      Value,
      Comments,
      Solution
    };

  std::string maxvariable;

  ClaspResultGrammar(std::size_t maxint)
  {
    std::ostringstream oss;
    oss << maxint;
    maxvariable = oss.str();
  }
  
  // S = ScannerT
  template<typename S>
  struct definition
  {
    // shortcut
    typedef BOOST_SPIRIT_CLASSIC_NS :: parser_context<> Ctx;
    template<int Tag> struct tag : public BOOST_SPIRIT_CLASSIC_NS :: parser_tag<Tag> {};
    
    /// copy ctor
    definition(ClaspResultGrammar const& self);

    BOOST_SPIRIT_CLASSIC_NS :: rule< S, Ctx, tag<Root> > const&
    start() const
    { return root; }
    
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Variable> > var;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Sentinel> > sentinel;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Neg> >      neg;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Literal> >  literal;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Value> >    value;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Comments> > comments;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Solution> > solution;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Root> >     root;
  };
};

} // namespace dmcs

#endif // CLASP_RESULT_GRAMMAR_H

#include "ClaspResultGrammar.tcc"

// Local Variables:
// mode: C++
// End:
