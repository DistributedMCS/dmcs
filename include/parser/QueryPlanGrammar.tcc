#ifndef __QUERY_PLAN_GRAMMAR_TCC__
#define __QUERY_PLAN_GRAMMAR_TCC__

#include <boost/spirit/include/qi.hpp>

#include "mcs/QueryPlan.h"

/////////////////////////////////////////////////////////////////
// Skipper //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator>
SkipperGrammar<Iterator>::SkipperGrammar() : SkipperGrammar::base_type(ws)
{
  using namespace boost::spirit;
  ws = ascii::space
       | qi::lexeme[ qi::char_('#') > *(qi::char_ - qi::eol) ];

  #ifdef BOOST_SPIRIT_DEBUG_WS
  BOOST_SPIRIT_DEBUG_NODE(ws);
  #endif
};


/////////////////////////////////////////////////////////////////
// QueryPlanGrammarBase /////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator, typename Skipper>
QueryPlanGrammarBase<Iterator, Skipper>::QueryPlanGrammarBase(QueryPlanGrammarSemantics &sem)
  : sem(sem)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  typedef QueryPlanGrammarSemantics Sem;

  using qi::lit;

  ident 
    = qi::lexeme[ ascii::lower >> *(ascii::alnum | qi::char_('_')) ];

  constants 
    = ( lit("Constants") >> lit(':') >> lit('[') >> ident % lit(',') >> lit(']') );// [Sem::constantList(sem)];

  start 
    = constants;
}

#endif // __QUERY_PLAN_GRAMMAR_TCC__

// Local Variables:
// mode: C++
// End:
