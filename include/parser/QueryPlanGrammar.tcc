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


struct lazy_get_impl
{
  template<typename X>
  struct result;

  template<typename T>
  struct result<boost::optional<T> >
  {
    typedef T type;
  };

  template<typename T>
  T operator()(const boost::optional<T>& optarg) const
  {
    return optarg.get();
  }
};

phoenix::function<lazy_get_impl> lazy_get;


/////////////////////////////////////////////////////////////////
// QueryPlanGrammarBase semantic processors /////////////////////
/////////////////////////////////////////////////////////////////
template<>
struct sem<QueryPlanGrammarSemantics::setContextID>
{
  void operator()(QueryPlanGrammarSemantics &mgr, int source, const boost::spirit::unused_type target)
  {
    std::cout << "sem<QueryPlanGrammarSemantics::setContextID>: s = " << source << std::endl;
  }
};


template<>
struct sem<QueryPlanGrammarSemantics::setConstantList>
{
  void operator()(QueryPlanGrammarSemantics &mgr, ConstantList source, const boost::spirit::unused_type target)
  {
    std::cout << "sem<QueryPlanGrammarSemantics::setConstantList>: s = " << std::endl;
    for (ConstantList::const_iterator it = source.begin(); it != source.end(); ++it)
      {
	std::cout << *it << " ";
      }
    std::cout << std::endl;
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
    = (
       lit('[') >> contextQueryPlan % ',' >> -(lit(',')) >> lit(']')
      );

  contextQueryPlan
    = lit('{') >>
    //    lit("ContextId") >> lit(':') >> int_ [Sem::setContextID(sem)] >> lit(',') >>
    //    lit("HostName")  >> lit(':') >> hostName >> lit(',') >>
    //    lit("Port")      >> lit(':') >> int_ >> lit(',') >>
    //    (-lit("Constants") >> lit(':') >> constants [Sem::setConstantList(sem)] >> lit(',')) >>
    lit('}');

  ident 
    = qi::lexeme[ ascii::lower >> *(ascii::alnum | qi::char_('_')) ];

  constants 
    = lit('[') >> ident % lit(',') >> lit(']') ;// [Sem::constantList(sem)];

  ///TODO: grammar for URL. Now simplified by putting hostname between the quotes
  hostName = lit('"') >> qi::lexeme[*(ascii::char_ - '"')] >> lit('"');

  id_with_ground_tuple 
    = uint_ >> lit(':') >> lit('[') >> ident % ',' >> lit(']');


  /*signature 
    = lit('{') >> 
    (id_with_ground_tuple % ',') [Sem::registerAndInsertIntoBeliefSet(sem)] >> 
    -(lit(',')) >> lit('}');*/
  

  BOOST_SPIRIT_DEBUG_NODE(start);
  BOOST_SPIRIT_DEBUG_NODE(constants);
}

} // namespace dmcs

#endif // __QUERY_PLAN_GRAMMAR_TCC__

// Local Variables:
// mode: C++
// End:
