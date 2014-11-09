#ifndef __BRIDGE_RULE_GRAMMAR_TCC__
#define __BRIDGE_RULE_GRAMMAR_TCC__

#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_DEBUG_WS

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp> 

#include "mcs/BeliefStateOffset.h"

namespace dmcs {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

/////////////////////////////////////////////////////////////////
// BridgeRuleGrammarBase semantic processors ////////////////////
/////////////////////////////////////////////////////////////////

template<>
struct sem<BridgeRuleGrammarSemantics::passPredToBelief>
{
  void operator()(BridgeRuleGrammarSemantics &mgr,
		  const fusion::vector2<std::string, std::vector<std::string> > &source,
		  std::string &target)
  {
    std::string pred_name = fusion::at_c<0>(source);
    std::vector<std::string> terms = fusion::at_c<1>(source);

    target = pred_name;
    if (!terms.empty())
      {
	target += "(";
	std::vector<std::string>::const_iterator endIt = terms.end();
	endIt--;
	for (std::vector<std::string>::const_iterator it = terms.begin(); it != endIt; ++it)
	  target = target + (*it) + ",";
	target = target + (*endIt) + ")";
      }
  }
};


template<>
struct sem<BridgeRuleGrammarSemantics::passIdentToBelief>
{
  void operator()(BridgeRuleGrammarSemantics &mgr,
		  const std::string &source,
		  std::string &target)
  {
    target = source;
  }
};


template<>
struct sem<BridgeRuleGrammarSemantics::getBridgeAtom>
{
  void operator()(BridgeRuleGrammarSemantics &mgr,
		  const fusion::vector2<std::size_t, const std::string&> &source,
		  ID &target)
  {
    std::size_t ctx_id = fusion::at_c<0>(source);
    const std::string& belief = fusion::at_c<1>(source);
    const ContextQueryPlan& qp = mgr.m_QueryPlan->find(ctx_id)->second;

    if (ctx_id == mgr.m_CtxID)
      target = qp.localSignature->getIDByString(belief);
    else
      {
	target = qp.groundInputSignature->getIDByString(belief);
	std::set<std::size_t>::const_iterator it = mgr.m_NeighborIDs.find(ctx_id);
	if (it == mgr.m_NeighborIDs.end())
	  {
	    mgr.m_NeighborIDs.insert(ctx_id);

	    std::stringstream str_port;
	    str_port << qp.port;

	    NewNeighborPtr neighbor(new NewNeighbor(ctx_id, mgr.m_NeighborOffset, qp.hostname, str_port.str()));
	    mgr.m_Neighbors->push_back(neighbor);
	    mgr.m_NeighborOffset++;
	  }
      }
  }
};


template<>
struct sem<BridgeRuleGrammarSemantics::getBridgeLiteral>
{
  void operator()(BridgeRuleGrammarSemantics &mgr,
		  const fusion::vector2<boost::optional<std::string>, ID> &source,
		  ID &target)
  {
    bool isNaf = !!fusion::at_c<0>(source);
    target = ID::literalFromBelief(fusion::at_c<1>(source), isNaf);
  }
};


template<>
struct sem<BridgeRuleGrammarSemantics::getBridgeRule>
{
  void operator()(BridgeRuleGrammarSemantics &mgr,
		  const fusion::vector2<const std::string&, std::vector<ID> > &source,
		  boost::spirit::unused_type target)
  {
    const std::string& head = fusion::at_c<0>(source);
    const ContextQueryPlan& qp = mgr.m_QueryPlan->find(mgr.m_CtxID)->second;
    ID head_id = qp.localSignature->getIDByString(head);

    const Tuple& body = fusion::at_c<1>(source);

    dmcs::BridgeRule r(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, head_id, body);
    mgr.m_BridgeRules->storeAndGetID(r);
  }
};

/////////////////////////////////////////////////////////////////
// BridgeRuleGrammarBase ////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator, typename NewSkipper>
BridgeRuleGrammarBase<Iterator, NewSkipper>::BridgeRuleGrammarBase(BridgeRuleGrammarSemantics &sem)
  : sem(sem)
{
  typedef BridgeRuleGrammarSemantics Sem;

  using qi::lit;
  using qi::eps;
  using qi::_1;
  using qi::int_;
  using qi::uint_;
  using qi::_val;
  using qi::char_;
  using qi::raw;
  using qi::lexeme;
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
    = predicate [ Sem::passPredToBelief(sem) ]
    | ident [ Sem::passIdentToBelief(sem) ];

  bridgeAtom
    = ( lit('(') >> uint_ >> lit(':') >> belief >> lit(')') ) [ Sem::getBridgeAtom(sem) ];

  bridgeLiteral 
    = ( -qi::string("not") >> bridgeAtom ) [ Sem::getBridgeLiteral(sem) ];

  bridgeRule
    = ( belief 
	>> qi::lit(":-")
	>> ( bridgeLiteral % qi::char_(',') )
	>> qi::lit('.')
	) [ Sem::getBridgeRule(sem) ];

  start = *bridgeRule;
}

} // namespace dmcs

#endif // __BRIDGE_RULE_GRAMMAR_TCC__

// Local Variables:
// mode: C++
// End:
