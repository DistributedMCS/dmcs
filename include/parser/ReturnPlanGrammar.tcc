#ifndef __RETURN_PLAN_GRAMMAR_TCC__
#define __RETURN_PLAN_GRAMMAR_TCC__

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
// QueryPlanGrammarBase semantic processors /////////////////////
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// ReturnPlanGrammarBase ////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator, typename NewSkipper>
ReturnPlanGrammarBase<Iterator, NewSkipper>::ReturnPlanGrammarBase(ReturnPlanGrammarSemantics &sem)
  : sem(sem)
{
  typedef ReturnPlanGrammarSemantics Sem;

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
      = ( lit('[') >> (returnPlan % ',' >> -(lit(',')) >> lit(']') ) );

    returnPlan 
      = lit('{') >>
        lit("ContextId")       >> lit(':') >> uint_ >> lit(',') >>
        lit("ReturnSignature") >> lit(':') >>
        returnSignature >> 
        lit('}');

    returnSignature 
      = lit('[') >> (returnBeliefs) % lit(',') >> -(lit(',')) >> lit(']');

    returnBeliefs 
      = lit('{') >>
        lit("ContextId")     >> lit(':') >> uint_ >> lit(',') >>
        lit("ReturnBeliefs") >> lit(':') >> 
        lit('[') >> uint_ % lit(',') >> lit(']') >> 
        lit('}');
}


} // namespace dmcs

#endif // __RETURN_PLAN_GRAMMAR_TCC__
