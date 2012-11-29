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
 * @file   NewDLVResultParser.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Nov  29 18:03:26 2012
 * 
 * @brief  
 * 
 * 
 */

#undef BOOST_SPIRIT_DEBUG
#undef BOOST_SPIRIT_DEBUG_WS

#include "mcs/BeliefStateOffset.h"
#include "parser/NewDLVResultParser.h"

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


struct HandleLiteral
{
  HandleLiteral(SemState& s) : s(s) { }

  template<typename SourceAttributes, typename Context>
  void
  operator()(const SourceAttributes& source,
	     Context& ctx,
	     boost::spirit::qi::unused_type) const
  {
    const boost::fusion::vector2<boost::optional<char>, std::string>& attr = source;

    bool strong_neg = boost::fusion::at_c<0>(attr);
    const std::string& belief_text = boost::fusion::at_c<1>(attr);
    ID belief_id = s.btab->getIDByString(belief_text);
    assert (belief_id != ID_FAIL);
    
    if (strong_neg)
      {
	s.current->set(s.ctx_id, 
		       belief_id.address,
		       BeliefStateOffset::instance()->getStartingOffsets(),
		       NewBeliefState::DMCS_FALSE);
      }
    else
      {
	s.current->set(s.ctx_id, 
		       belief_id.address,
		       BeliefStateOffset::instance()->getStartingOffsets());
      }
  }

  SemState& s;
};



template<typename Iterator, typename Skipper>
struct DLVResultGrammar : qi::grammar<Iterator, Skipper>
{
  DLVResultGrammar(SemState& state) : DLVResultGrammar::base_type(dlvline)
  {
    using qi::int_;
    using qi::_val;
    using qi::_1;
    using qi::lexeme;
    using qi::raw;
    using qi::char_;
    using qi::omit;
    using qi::lit;

    number = 
      lexeme[ char_('0') ]
      | lexeme[ char_('1', '9') >> *(char_('0', '9')) ];

    ident =
      lexeme[char_('"') >> *(char_ - '"') >> char_('"')]
      | (ascii::lower >> *(ascii::alnum|char_('_')));
    
    ///TODO: When raw[] is not used, we won't get from 2nd arguments in the facts. 
    ///Something wrong with boost!!! try not to use raw[] with newer version of boost!
    fact = 
      raw[ident >> -( char_('(') >> (ident | number) >> *(char_(',') >> (ident | number)) >> char_(')') )];
    
    mlit = ( -char_('-') >> fact >> lit(',') ) [ HandleLiteral(state) ];
    flit = ( -char_('-') >> fact >> lit('}') ) [ HandleLiteral(state) ];
    
    answerset
      = (lit('{') >> '}')
      | (lit('{') >> *(mlit) >> flit);
    
    costline
      = lit("Cost") >> +(ascii::alnum | char_("[]<>():"));
    
    dlvline
      = (-lit("Best model:") >> answerset)
      |
      costline;
  }

  qi::rule<Iterator, Skipper>                costline, answerset, mlit, flit, dlvline;
  qi::rule<Iterator, std::string(), Skipper> ident, fact, number;

};


} // namespace

/*******************************************************************************************/

namespace dmcs {

NewBeliefState* NewDLVResultParser::parseString(const std::string& instr)
{
  state.reset_current();

  std::string::const_iterator begIt = instr.begin();
  std::string::const_iterator endIt = instr.end();

  typedef SkipperGrammar<std::string::const_iterator> Skipper;

  Skipper skipper;
  
  DLVResultGrammar<std::string::const_iterator, Skipper> grammar(state);

  bool r = qi::phrase_parse(begIt, endIt, grammar, skipper);

  if (r && begIt == endIt)
    {
      return state.current;
      //std::cerr << "Bridge rules parsing succeeded" << std::endl;
    }
  else
    {
      //std::cerr << "Bridge rules parsing failed" << std::endl;
      throw std::runtime_error("Bridge rules parsing failed");
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
