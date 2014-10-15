#ifndef __QUERY_PLAN_GRAMMAR_TCC__
#define __QUERY_PLAN_GRAMMAR_TCC__

#include <boost/spirit/include/qi.hpp>

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

#endif // __QUERY_PLAN_GRAMMAR_TCC__
