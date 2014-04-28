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
 * @file   ReturnPlanParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Oct  22 16:26:26 2012
 * 
 * @brief  
 * 
 * 
 */

#undef BOOST_SPIRIT_DEBUG
#undef BOOST_SPIRIT_DEBUG_WS

#include "mcs/BeliefStateOffset.h"
#include "parser/ReturnPlanParser.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp> 

#include <istream>
#include <fstream>

namespace {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

using namespace dmcs;


struct SemState
{
  SemState()
    : return_plan(new ReturnPlanMap)
  { }

  ReturnPlanMapPtr return_plan;
  std::size_t      current_parent_id;
  NewBeliefState*  current_belief_state;
};


struct InsertIntoMap
{
  SemState& s;

  InsertIntoMap(SemState& s)
    : s(s)
  { }

  template<typename SourceAttributes, typename Ctx>
  void operator()(const SourceAttributes& source,
		  Ctx& ctx,
		  boost::spirit::qi::unused_type) const
  {
    s.return_plan->insert(std::make_pair(s.current_parent_id, s.current_belief_state));
  }
};


struct GetParentId
{
  SemState& s;

  GetParentId(SemState& s)
    : s(s)
  { }

  template<typename SourceAttributes, typename Ctx>
  void operator()(const SourceAttributes& source,
		  Ctx& ctx,
		  boost::spirit::qi::unused_type) const
  {
    const unsigned int parent_id = source;
    s.current_parent_id = parent_id;
    s.current_belief_state = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
						BeliefStateOffset::instance()->SIZE_BS());
    s.current_belief_state->setEpsilon(BeliefStateOffset::instance()->getStartingOffsets());
  }
};


struct ActivateBits
{
  SemState& s;

  ActivateBits(SemState& s)
    : s(s)
  { }

  template<typename SourceAttributes, typename Ctx>
  void operator()(const SourceAttributes& source,
		  Ctx& ctx,
		  boost::spirit::qi::unused_type) const
  {
    const fusion::vector2<unsigned int, std::vector<unsigned int> >& input = source;
    const unsigned int ctx_id = fusion::at_c<0>(input);
    const std::vector<unsigned int>& local_ids = fusion::at_c<1>(input);

    for (std::vector<unsigned int>::const_iterator it = local_ids.begin();
	 it != local_ids.end(); ++it)
      {
	s.current_belief_state->set(ctx_id, *it, 
				    BeliefStateOffset::instance()->getStartingOffsets(),
				    NewBeliefState::DMCS_TRUE);

      }
  }  
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


template<typename Iterator, typename Skipper>
struct ReturnPlanGrammar : qi::grammar<Iterator, Skipper>
{
  ReturnPlanGrammar(SemState& state) : ReturnPlanGrammar::base_type(start)
  {
    using qi::lit;
    using qi::eps;
    using qi::_1;
    using qi::int_;
    using qi::uint_;
    using qi::_val;
    using qi::char_;
    using namespace qi::labels;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::insert;
    using phoenix::at_c;

    start = ( 
	     lit('[') >> (return_plan [InsertIntoMap(state)]) % ',' >> -(lit(',')) >> lit(']')
	    );

    return_plan = lit('{') >>
      lit("ContextId") >> ':' >> uint_ [GetParentId(state)] >> lit(',') >>
      lit("ReturnSignature") >> ':' >>
      return_signature >> 
      lit('}');

    return_signature =
      lit('[') >> (return_beliefs [ActivateBits(state)]) % ',' >> -(lit(',')) >> lit(']');

    return_beliefs = lit('{') >>
      lit("ContextId") >> ':' >> uint_ >> lit(',') >>
      lit("ReturnBeliefs") >> ':' >> 
      lit('[') >> uint_ % ',' >> lit(']') >> 
      lit('}');
  }

  qi::rule<Iterator, Skipper> start;
  qi::rule<Iterator, Skipper> return_plan;
  qi::rule<Iterator, Skipper> return_signature;
  qi::rule<Iterator, fusion::vector2<unsigned int, std::vector<unsigned int> >(), Skipper> return_beliefs;
};


} // namespace

/*******************************************************************************************/

namespace dmcs {

ReturnPlanMapPtr 
ReturnPlanParser::parseStream(std::istream& in)
{
  std::ostringstream buf;
  std::string line;

  while (!in.eof())
    {
      std::getline(in, line);
      buf << line << std::endl;
      //std::cerr << "Read >>" << line << "<<" << std::endl;
    }

  if (in.fail()) in.clear();

  std::string input = buf.str();
  return parseString(input);
}



ReturnPlanMapPtr 
ReturnPlanParser::parseFile(const std::string& infile)
{
  std::ifstream ifs;

  ifs.open(infile.c_str());
  if (!ifs.is_open())
    {
      std::ostringstream oss;
      oss << "File " << infile << " not found!";
      throw std::runtime_error(oss.str());
    }
  else
    {
      return parseStream(ifs);
    }
}



ReturnPlanMapPtr 
ReturnPlanParser::parseString(const std::string& instr)
{
  std::string::const_iterator begIt = instr.begin();
  std::string::const_iterator endIt = instr.end();


  typedef SkipperGrammar<std::string::const_iterator> Skipper;

  Skipper skipper;
  SemState state;


  ReturnPlanGrammar<std::string::const_iterator, Skipper> grammar(state);

  bool r = qi::phrase_parse(begIt, endIt, grammar, skipper);

  if (r && begIt == endIt)
    {
      //std::cerr << "Return plan parsing succeeded" << std::endl;
      return state.return_plan;
    }
  else
    {
      //std::cerr << "Return plan parsing failed" << std::endl;
      throw std::runtime_error("Return plan parsing failed");
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
