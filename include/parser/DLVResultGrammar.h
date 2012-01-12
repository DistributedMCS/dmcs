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
 * @file   DLVResultGrammar.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @author Peter Schüller <ps@kr.tuwien.ac.at>
 * @date   Thu Jan  12 12:15:14 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef DLV_RESULT_GRAMMAR_H
#define DLV_RESULT_GRAMMAR_H

#include <boost/optional.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/include/pair.hpp>

#include <sstream>
#include <iostream>

#include "parser/ParserState.h"

namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;

namespace dmcs {

struct handle_literal
{
  handle_literal(ParserState& state);

  void operator()(boost::fusion::vector2<boost::optional<char>,
				      std::string>& attr, 
		  qi::unused_type, qi::unused_type) const;

  ParserState& state;
};



struct handle_finished_answerset
{
  handle_finished_answerset(ParserState& state);

  void operator()(qi::unused_type, qi::unused_type, qi::unused_type) const;

  ParserState& state;
};



template<typename Iterator>
struct DLVResultGrammar : public qi::grammar<Iterator, ascii::space_type>
{
  DLVResultGrammar(ParserState& state) : DLVResultGrammar::base_type(dlvline), state(state)
  {
    using spirit::int_;
    using spirit::_val;
    using spirit::_1;
    using qi::lexeme;
    using qi::char_;
    using qi::omit;
    using qi::lit;
    
    ident = lexeme[char_('"') > *(char_ - '"') > char_('"')]
      | lexeme[ascii::lower > *(ascii::alnum|char_('_'))];

    groundterm = int_ | ident;

    // char_ synthesizes a char attribute!
    fact = ( ident > -params );

    literal = ( -char_('-') > fact ) [ handle_literal(state) ] ;

    params
      %= '(' > groundterm > *(',' > groundterm) > ')';

    answerset
      = (lit('{') >> '}') [ handle_finished_answerset(state) ]
      | (lit('{') > literal > *(',' > literal) > lit('}') [ handle_finished_answerset(state) ]);

    costline
      = lit("Cost") > +(ascii::alnum|char_("[]<>():"));

    dlvline
      = (-lit("Best model:") >> answerset)
      |
      costline;
    
#ifdef BOOST_SPIRIT_DEBUG
    BOOST_SPIRIT_DEBUG_NODE(dlvline);
    BOOST_SPIRIT_DEBUG_NODE(answerset);
    BOOST_SPIRIT_DEBUG_NODE(costline);
    BOOST_SPIRIT_DEBUG_NODE(literal);
    BOOST_SPIRIT_DEBUG_NODE(fact);
    BOOST_SPIRIT_DEBUG_NODE(groundterm);
    BOOST_SPIRIT_DEBUG_NODE(ident);
#endif
  }
  
  qi::rule<Iterator, ascii::space_type>                  dlvline, answerset, literal, costline, groundterm, params;
  //qi::rule<Iterator, ID(), ascii::space_type>            groundterm;
  qi::rule<Iterator, std::string(), ascii::space_type>   ident, fact;
  //qi::rule<Iterator, Tuple(), ascii::space_type>         params;
  
  ParserState& state;
};

} // namespace dmcs

#endif // DLV_RESULT_GRAMMAR_H


// Local Variables:
// mode: C++
// End:
