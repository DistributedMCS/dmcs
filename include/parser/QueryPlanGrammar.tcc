#ifndef __QUERY_PLAN_GRAMMAR_TCC__
#define __QUERY_PLAN_GRAMMAR_TCC__

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

#include "mcs/QueryPlan.h"

namespace dmcs {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

/////////////////////////////////////////////////////////////////
// Skipper //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator>
NewSkipperGrammar<Iterator>::NewSkipperGrammar() : NewSkipperGrammar::base_type(ws)
{
  using namespace boost::spirit;
  ws = ascii::space
       | qi::lexeme[ qi::char_('#') > *(qi::char_ - qi::eol) ];

  #ifdef BOOST_SPIRIT_DEBUG_WS
  BOOST_SPIRIT_DEBUG_NODE(ws);
  #endif
};


/////////////////////////////////////////////////////////////////
// QueryPlanGrammarBase semantic processors /////////////////////
/////////////////////////////////////////////////////////////////
template<>
struct sem<QueryPlanGrammarSemantics::setContextID>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const int &source, 
		  const boost::spirit::unused_type target)
  {
    ContextQueryPlanPtr& currentQP = mgr.m_CurrentQueryPlan;
    currentQP = ContextQueryPlanPtr(new ContextQueryPlan);
    currentQP->ctx = source;  
  }
};


template<>
struct sem<QueryPlanGrammarSemantics::setHostName>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const std::string &source, 
		  const boost::spirit::unused_type target)
  {
    ContextQueryPlanPtr& currentQP = mgr.m_CurrentQueryPlan;
    currentQP->hostname = source;
  }
};



template<>
struct sem<QueryPlanGrammarSemantics::setPort>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const int &source, 
		  const boost::spirit::unused_type target)
  {
    ContextQueryPlanPtr& currentQP = mgr.m_CurrentQueryPlan;
    currentQP->port = source;
  }
};



template<>
struct sem<QueryPlanGrammarSemantics::setConstantList>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const ConstantList &source, 
		  const boost::spirit::unused_type target)
  {
    ContextQueryPlanPtr& currentQP = mgr.m_CurrentQueryPlan;
    currentQP->constants = ConstantListPtr(new ConstantList(source));
  }
};




template<>
struct sem<QueryPlanGrammarSemantics::registerAndInsertIntoBeliefSet>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const std::vector<boost::fusion::vector2<IDAddress, std::vector<std::string> > > &source, 
		  BeliefTablePtr &target)
  {
  }
};



template<>
struct sem<QueryPlanGrammarSemantics::setLocalSignature>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const BeliefTablePtr &source, 
		  boost::spirit::qi::unused_type target)
  {
  }
};



template<>
struct sem<QueryPlanGrammarSemantics::setInputSignature>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const BeliefTablePtr &source, 
		  boost::spirit::qi::unused_type target)
  {
  }
};



template<>
struct sem<QueryPlanGrammarSemantics::insertIntoMap>
{
  void operator()(QueryPlanGrammarSemantics &mgr, 
		  const boost::spirit::qi::unused_type &source,
		  boost::spirit::qi::unused_type target)
  {
    ContextQueryPlanMapPtr &qpm = mgr.m_QueryPlanMap;
    ContextQueryPlanPtr &currentQP = mgr.m_CurrentQueryPlan;

    if (!qpm) qpm.reset(new ContextQueryPlanMap);
    qpm->insert(std::make_pair(currentQP->ctx, *currentQP));
  }
};



/////////////////////////////////////////////////////////////////
// QueryPlanGrammarBase /////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator, typename NewSkipper>
QueryPlanGrammarBase<Iterator, NewSkipper>::QueryPlanGrammarBase(QueryPlanGrammarSemantics &sem)
  : sem(sem)
{
  typedef QueryPlanGrammarSemantics Sem;

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

  start 
    = ( lit('[') >> contextQueryPlan % ',' >> -(lit(',')) >> lit(']') );

  contextQueryPlan
    = (lit('{') >>
       lit("ContextId")      >> lit(':') >> int_      [Sem::setContextID(sem)]      >> lit(',') >>
       lit("HostName")       >> lit(':') >> hostName  [Sem::setHostName(sem)]       >> lit(',') >>
       lit("Port")           >> lit(':') >> int_      [Sem::setPort(sem)]           >> lit(',') >>
    (-(lit("Constants")      >> lit(':') >> constants [Sem::setConstantList(sem)]   >> lit(',') )) >>
    (-(lit("LocalSignature") >> lit(':') >> signature [Sem::setLocalSignature(sem)] >> lit(',') )) >>
    (-(lit("InputSignature") >> lit(':') >> signature [Sem::setInputSignature(sem)] >> lit(',') )) >>
       lit('}')) [Sem::insertIntoMap(sem)];

  ident 
    = qi::lexeme[ ascii::lower >> *(ascii::alnum | qi::char_('_')) ];

  constants 
    = lit('[') >> ident % lit(',') >> lit(']') ;// [Sem::constantList(sem)];

  ///TODO: grammar for URL. Now simplified by putting hostname between the quotes
  hostName = lit('"') >> qi::lexeme[*(ascii::char_ - '"')] >> lit('"');

  id_with_ground_tuple 
    = uint_ >> lit(':') >> lit('[') >> ident % ',' >> lit(']');

  signature 
    = lit('{') >> 
    (id_with_ground_tuple % ',') [Sem::registerAndInsertIntoBeliefSet(sem)] >> 
    -(lit(',')) >> lit('}');
  
  #ifdef BOOST_SPIRIT_DEBUG
  BOOST_SPIRIT_DEBUG_NODE(start);
  BOOST_SPIRIT_DEBUG_NODE(constants);
  #endif
}

} // namespace dmcs

#endif // __QUERY_PLAN_GRAMMAR_TCC__

// Local Variables:
// mode: C++
// End:
