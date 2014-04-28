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

#include "parser/BridgeRuleParser.h"
#include "mcs/BridgeRuleTable.h"
#include "mcs/NewNeighbor.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

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
  SemState(const ContextQueryPlanMapPtr& queryplan,
	   const std::size_t ctx_id)
    : bridge_rules(new BridgeRuleTable),
      neighbors(new NewNeighborVec),
      neighbor_offset(0),
      queryplan(queryplan),
      ctx_id(ctx_id)
  { }

  BridgeRuleTablePtr bridge_rules;
  NewNeighborVecPtr neighbors;
  std::size_t neighbor_offset;
  std::set<std::size_t> neighbor_ids;
  const ContextQueryPlanMapPtr queryplan;
  const std::size_t ctx_id;
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

struct PassIdentToBelief
{
  template<typename SourceAttributes, typename Context>
  void
  operator()(const SourceAttributes& source,
	     Context& ctx,
	     boost::spirit::qi::unused_type) const
  {
    const std::string& ident = source;
    std::string& belief = boost::fusion::at_c<0>(ctx.attributes);
    belief = source;
  }
};

struct PassPredToBelief
{
  template<typename SourceAttributes, typename Context>
  void
  operator()(const SourceAttributes& source,
	     Context& ctx,
	     boost::spirit::qi::unused_type) const
  {
    const fusion::vector2<std::string, std::vector<std::string> >& input = source;
    std::string pred_name = fusion::at_c<0>(input);
    std::vector<std::string> terms = fusion::at_c<1>(input);
    std::string& belief = fusion::at_c<0>(ctx.attributes);
    belief = pred_name;
    if (!terms.empty())
      {
	belief = belief + "(";
	std::vector<std::string>::const_iterator endIt = terms.end();
	endIt--;
	for (std::vector<std::string>::const_iterator it = terms.begin(); it != endIt; ++it)
	  belief = belief + (*it) + ",";
	belief = belief + (*endIt) + ")";
      }
  }
};

struct GetBridgeAtom
{
  GetBridgeAtom(SemState& s) : s(s) { }

  template<typename SourceAttributes, typename Context>
  void
  operator()(const SourceAttributes& source,
	     Context& ctx,
	     boost::spirit::qi::unused_type) const
  {
    const fusion::vector2<std::size_t, const std::string&>& input = source;
    dmcs::ID& id = fusion::at_c<0>(ctx.attributes);

    std::size_t ctx_id = fusion::at_c<0>(input);
    const std::string& belief = fusion::at_c<1>(input);
    const ContextQueryPlan& qp = s.queryplan->find(ctx_id)->second;

    if (ctx_id == s.ctx_id)
      id = qp.localSignature->getIDByString(belief);
    else
      {
	id = qp.groundInputSignature->getIDByString(belief);
	std::set<std::size_t>::const_iterator it = s.neighbor_ids.find(ctx_id);
	if (it == s.neighbor_ids.end())
	  {
	    s.neighbor_ids.insert(ctx_id);

	    std::stringstream str_port;
	    str_port << qp.port;

	    NewNeighborPtr neighbor(new NewNeighbor(ctx_id, s.neighbor_offset, qp.hostname, str_port.str()));
	    s.neighbors->push_back(neighbor);
	    s.neighbor_offset++;
	  }
      }
  }

  SemState& s;
};

struct GetBridgeLiteral
{
  template<typename SourceAttributes, typename Context>
  void
  operator()(const SourceAttributes& source,
	     Context& ctx,
	     boost::spirit::qi::unused_type) const
  {
    const fusion::vector2<boost::optional<std::string>, dmcs::ID>& input = source;
    dmcs::ID& id = fusion::at_c<0>(ctx.attributes);

    bool isNaf = !!fusion::at_c<0>(input);
    id = dmcs::ID::literalFromBelief(fusion::at_c<1>(input), isNaf);
  }
};

struct GetBridgeRule
{
  GetBridgeRule(SemState& s) : s(s) { }

  template<typename SourceAttributes, typename Context>
  void
  operator()(const SourceAttributes& source,
	     Context& ctx,
	     boost::spirit::qi::unused_type) const
  {
    const fusion::vector2<const std::string&, std::vector<dmcs::ID> >& input = source;

    const std::string& head = fusion::at_c<0>(input);
    const ContextQueryPlan& qp = s.queryplan->find(s.ctx_id)->second;
    dmcs::ID head_id = qp.localSignature->getIDByString(head);

    const Tuple& body = fusion::at_c<1>(input);

    dmcs::BridgeRule r(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, head_id, body);
    s.bridge_rules->storeAndGetID(r);
  }

  SemState& s;
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
    using qi::char_;
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

    belief 
      = predicate [ PassPredToBelief() ]
      | ident [ PassIdentToBelief() ];

    bridge_atom
      = ( lit('(') >> uint_ >> lit(':') >> belief >> lit(')') ) [ GetBridgeAtom(state) ];

    bridge_literal 
      = ( -qi::string("not") >> bridge_atom ) [ GetBridgeLiteral() ];

    bridge_rule
      = ( belief 
	  >> qi::lit(":-")
	  >> ( bridge_literal % qi::char_(',') )
	  >> qi::lit('.')
	  ) [ GetBridgeRule(state) ];

    start = *bridge_rule;
  }

  qi::rule<Iterator, std::string(), Skipper> ident;
  qi::rule<Iterator, std::vector<std::string>(), Skipper> terms;
  qi::rule<Iterator, fusion::vector2<std::string, std::vector<std::string> >(), Skipper> predicate;
  qi::rule<Iterator, std::string(), Skipper> belief;
  qi::rule<Iterator, dmcs::ID(), Skipper> bridge_atom;
  qi::rule<Iterator, dmcs::ID(), Skipper> bridge_literal;
  qi::rule<Iterator, dmcs::ID(), Skipper> bridge_rule;
  qi::rule<Iterator, Skipper> start;
};


} // namespace 

/*******************************************************************************************/

namespace dmcs {

BridgeRuleParserReturnVal
BridgeRuleParser::parseFile(const std::string& infile,
			    ContextQueryPlanMapPtr& queryplan,
			    const std::size_t ctx_id)
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
      return parseStream(ifs, queryplan, ctx_id);
    }
}



BridgeRuleParserReturnVal
BridgeRuleParser::parseStream(std::istream& in,
			      ContextQueryPlanMapPtr& queryplan,
			      const std::size_t ctx_id)
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
  return parseString(input, queryplan, ctx_id);
}



BridgeRuleParserReturnVal
BridgeRuleParser::parseString(const std::string& instr,
			      ContextQueryPlanMapPtr& queryplan,
			      const std::size_t ctx_id)
{
  std::string::const_iterator begIt = instr.begin();
  std::string::const_iterator endIt = instr.end();

  typedef SkipperGrammar<std::string::const_iterator> Skipper;

  Skipper skipper;
  SemState state(queryplan, ctx_id);
  BridgeRuleGrammar<std::string::const_iterator, Skipper> grammar(state);

  bool r = qi::phrase_parse(begIt, endIt, grammar, skipper);

  if (r && begIt == endIt)
    {
      //std::cerr << "Bridge rules parsing succeeded" << std::endl;
      BridgeRuleParserReturnVal ret_val(state.bridge_rules, state.neighbors);
      return ret_val;
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
