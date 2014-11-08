#ifndef __DLV_RESULT_GRAMMAR_TCC__
#define __DLV_RESULT_GRAMMAR_TCC__

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
// DLVResultGrammarBase semantic processors /////////////////////
/////////////////////////////////////////////////////////////////

template<>
struct sem<DLVResultGrammarSemantics::handleLiteral>
{
  void operator()(DLVResultGrammarSemantics &mgr,
		  const boost::fusion::vector2<boost::optional<char>, std::string> &source,
		  boost::spirit::unused_type target)
  {
    bool strong_neg = boost::fusion::at_c<0>(source);
    const std::string& belief_text = boost::fusion::at_c<1>(source);

    ID belief_id = mgr.m_BTab->getIDByString(belief_text);
    assert (belief_id != ID_FAIL);
    
    if (strong_neg)
      {
	mgr.m_ParsedResult->set(mgr.m_CtxID, 
				belief_id.address,
				BeliefStateOffset::instance()->getStartingOffsets(),
				NewBeliefState::DMCS_FALSE);
      }
    else
      {
	mgr.m_ParsedResult->set(mgr.m_CtxID, 
				belief_id.address,
				BeliefStateOffset::instance()->getStartingOffsets());
      }    
  }
};


/////////////////////////////////////////////////////////////////
// DLVResultGrammarBase /////////////////////////////////////////
/////////////////////////////////////////////////////////////////
template<typename Iterator, typename NewSkipper>
DLVResultGrammarBase<Iterator, NewSkipper>::DLVResultGrammarBase(DLVResultGrammarSemantics &sem)
  : sem(sem)
{
  typedef DLVResultGrammarSemantics Sem;

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

  number 
    = lexeme[ char_('0') ]
    | lexeme[ char_('1', '9') >> *(char_('0', '9')) ];
  
  ident 
    = lexeme[char_('"') >> *(char_ - '"') >> char_('"')]
    | (ascii::lower >> *(ascii::alnum|char_('_')));
  
  ///TODO: When raw[] is not used, we won't get from 2nd arguments in the facts. 
  ///Something wrong with boost!!! try not to use raw[] with newer version of boost!
  fact 
    = raw[ident >> -( char_('(') >> (ident | number) >> *(char_(',') >> (ident | number)) >> char_(')') )];
  
  mlit
    = ( -char_('-') >> fact >> lit(',') ) [ Sem::handleLiteral(sem) ];

  flit 
    = ( -char_('-') >> fact >> lit('}') ) [ Sem::handleLiteral(sem) ];
  
  answerset
    = (lit('{') >> '}')
    | (lit('{') >> *(mlit) >> flit);
  
  costline
    = lit("Cost") >> +(ascii::alnum|char_("[]<>():"));
  
  dlvline
    = (-lit("Best model:") >> answerset)
    |
    costline;
}


} // namespace dmcs

#endif // __DLV_RESULT_GRAMMAR_TCC__

// Local Variables:
// mode: C++
// End:
