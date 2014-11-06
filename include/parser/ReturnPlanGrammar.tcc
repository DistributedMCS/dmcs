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
// ReturnPlanGrammarBase semantic processors ////////////////////
/////////////////////////////////////////////////////////////////

template<>
struct sem<ReturnPlanGrammarSemantics::getCurrentParentID>
{
  void operator()(ReturnPlanGrammarSemantics &mgr, 
		  const std::size_t &source, 
		  const boost::spirit::unused_type target)
  {
    mgr.m_CurrentParentID = source;
    mgr.m_CurrentBeliefState = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
						  BeliefStateOffset::instance()->SIZE_BS());
    mgr.m_CurrentBeliefState->setEpsilon(BeliefStateOffset::instance()->getStartingOffsets());

  }
};



template<>
struct sem<ReturnPlanGrammarSemantics::activateBits>
{
  void operator()(ReturnPlanGrammarSemantics &mgr, 
		  const fusion::vector2<std::size_t, std::vector<std::size_t> > &source,
		  const boost::spirit::unused_type target)
  {
    const std::size_t ctx_id = fusion::at_c<0>(source);
    const std::vector<std::size_t> &local_ids = fusion::at_c<1>(source);
    for (std::vector<std::size_t>::const_iterator it = local_ids.begin();
	 it != local_ids.end(); ++it)
      {
	mgr.m_CurrentBeliefState->set(ctx_id, *it, 
				      BeliefStateOffset::instance()->getStartingOffsets(),
				      NewBeliefState::DMCS_TRUE);

      }
  }  
};



template<>
struct sem<ReturnPlanGrammarSemantics::insertIntoMap>
{
  void operator()(ReturnPlanGrammarSemantics &mgr, 
		  const boost::spirit::unused_type &source,
		  const boost::spirit::unused_type target)
  {
    ReturnPlanMapPtr &rpm = mgr.m_ParsedResult;
    if (!rpm) rpm.reset(new ReturnPlanMap);

    mgr.m_ParsedResult->insert(std::make_pair(mgr.m_CurrentParentID, mgr.m_CurrentBeliefState));
  }
};

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
      = ( lit('[') >> ( (returnPlan [Sem::insertIntoMap(sem)]) % ',' >> -(lit(',')) >> lit(']') ) );

    returnPlan 
      = lit('{') >>
      lit("ContextId")       >> lit(':') >> uint_ [Sem::getCurrentParentID(sem)] >> lit(',') >>
        lit("ReturnSignature") >> lit(':') >>
        returnSignature >> 
        lit('}');

    returnSignature 
      = lit('[') >> (returnBeliefs [Sem::activateBits(sem)]) % lit(',') >> -(lit(',')) >> lit(']');

    returnBeliefs 
      = lit('{') >>
        lit("ContextId")     >> lit(':') >> uint_ >> lit(',') >>
        lit("ReturnBeliefs") >> lit(':') >> 
        lit('[') >> uint_ % lit(',') >> lit(']') >> 
        lit('}');
}


} // namespace dmcs

#endif // __RETURN_PLAN_GRAMMAR_TCC__

// Local Variables:
// mode: C++
// End:
