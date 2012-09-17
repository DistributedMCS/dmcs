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
 * @file   BridgeRuleParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  17 16:08:24 2012
 * 
 * @brief  
 * 
 * 
 */

#undef BOOST_SPIRIT_DEBUG
#undef BOOST_SPIRIT_DEBUG_WS

#include "BridgeRuleParser.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp> 

namespace {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

using namespace dmcs;

struct SemState
{
  //BridgeRuleListPtr bridge_rules;
};

template<typename Iterator>
struct SkipperGrammar: boost::spirit::qi::grammar<Iterator>
{
  SkipperGrammar(): SkipperGrammar::base_type(ws)
  {
    using namespace boost::spirit;
    ws = ascii::space
       | qi::lexeme[ qi::char_('%') > *(qi::char_ - qi::eol) ];

    #ifdef BOOST_SPIRIT_DEBUG_WS
    BOOST_SPIRIT_DEBUG_NODE(ws);
    #endif
  }

  boost::spirit::qi::rule<Iterator> ws; 
};

struct TestOp
{
  template<typename SourceAttributes>
  void 
  operator()(boost::spirit::qi::unused_type,
	     const SourceAttributes& source,
	     boost::spirit::qi::unused_type) const
  {
    const std::vector<std::string>& v = source;
    //for (std::vector<std::string>::const_iterator it = source.begin(); it != source.end(); ++it)
    //  std::cout << *it << std::endl;
  }
};

template<typename Iterator, typename Skipper>
struct BridgeRuleGrammar : qi::grammar<Iterator, Skipper>
{
  BridgeRuleGrammar(SemState& state) : BridgeRuleGrammar::base_type(start)
  {
    using qi::lit;
    using qi::eps;
    using qi::_1;
    using qi::int_;
    using qi::uint_;
    using qi::_val;
    using namespace qi::labels;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::insert;
    using phoenix::at_c;

    ident 
      %= ( +(ascii::alnum) )
      | ( lit('"') >> qi::lexeme[*(ascii::char_ - '"')] >> lit('"') );

    terms 
      = ident % qi::lit(',');

    predicate 
      = ident >> lit('(') >> terms >> lit(')');

    

    start = predicate [ TestOp() ];
  }

  qi::rule<Iterator, std::string(), Skipper> ident;
  qi::rule<Iterator, std::vector<std::string>, Skipper> terms;
  qi::rule<Iterator, std::vector<std::string>, Skipper> predicate;
  qi::rule<Iterator, Skipper> start;
};


} // namespace 

/*******************************************************************************************/

namespace dmcs {

  /*
BridgeRuleListPtr
BridgeRuleParser::parseStream(std::istream& in)
{
}

BridgeRuleListPtr
BridgeRuleParser::parseFile(const std::string& infile)
{
}

BridgeRuleListPtr 
BridgeRuleParser::parseString(const std::string& instr)
{
}*/

bool
BridgeRuleParser::parseString(const std::string& instr)
{
  std::string::const_iterator begIt = instr.begin();
  std::string::const_iterator endIt = instr.end();

  typedef SkipperGrammar<std::string::const_iterator> Skipper;

  Skipper skipper;
  SemState state;
  BridgeRuleGrammar<std::string::const_iterator, Skipper> grammar(state);

  bool r = qi::phrase_parse(begIt, endIt, grammar, skipper);

  if (r && begIt == endIt)
    {
      std::cerr << "Bridge rules parsing succeeded" << std::endl;
      return true;
    }
  else
    {
      std::cerr << "Bridge rules parsing failed" << std::endl;
      throw std::runtime_error("Bridge rules parsing failed");
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
