#ifndef __RETURN_PLAN_GRAMMAR_HPP__
#define __RETURN_PLAN_GRAMMAR_HPP__

#include <boost/spirit/include/qi.hpp>

#include "mcs/NewBeliefState.h"
#include "parser/BaseParser.hpp"

namespace dmcs {

namespace fusion = boost::fusion;

class ReturnPlanGrammarSemantics
{
public:
  ReturnPlanMapPtr m_ParsedResult;
  std::size_t      m_CurrentParentID;
  NewBeliefState  *m_CurrentBeliefState;

public:
  ReturnPlanGrammarSemantics()
  { }

  #define DMCS_DEFINE_SEMANTIC_ACTION(name, targettype) \
    struct name: \
      SemanticActionBase<ReturnPlanGrammarSemantics, targettype, name> \
    { \
      name(ReturnPlanGrammarSemantics& mgr): name ::base_type(mgr) {} \
    };

  DMCS_DEFINE_SEMANTIC_ACTION(getCurrentParentID, boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(activateBits, boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(insertIntoMap, boost::spirit::unused_type);

  #undef DMCS_DEFINE_SEMANTIC_ACTION
};


//! basic ReturnPlanGrammar
template<typename Iterator, typename NewSkipper>
struct ReturnPlanGrammarBase
{
  ReturnPlanGrammarSemantics &sem;
  ReturnPlanGrammarBase(ReturnPlanGrammarSemantics&);

  // Helper struct for creating rule types
  template<typename Attrib=void, typename Dummy=void>
  struct Rule
  {
    typedef boost::spirit::qi::rule<Iterator, Attrib(), NewSkipper> type;
  };

  template<typename Dummy>
  struct Rule<void, Dummy>
  {
    typedef boost::spirit::qi::rule<Iterator, NewSkipper> type;
    // BEWARE: this is _not_ the same (!) as
    // typedef boost::spirit::qi::rule<Iterator, boost::spirit::unused_type, NewSkipper> type;
  };

  // Core grammar rules
  typename Rule<>::type start, returnPlan, returnSignature;
  typename Rule<fusion::vector2<std::size_t, std::vector<std::size_t> > >::type returnBeliefs;
};


template<typename Iterator, typename NewSkipper>
struct NewReturnPlanGrammar : 
  ReturnPlanGrammarBase<Iterator, NewSkipper>,
  boost::spirit::qi::grammar<Iterator, NewSkipper>
{
  typedef ReturnPlanGrammarBase<Iterator, NewSkipper> GrammarBase;
  typedef boost::spirit::qi::grammar<Iterator, NewSkipper> QiBase;

  NewReturnPlanGrammar(ReturnPlanGrammarSemantics& sem)
    : GrammarBase(sem),
      QiBase(GrammarBase::start)
  { }      
};


} // namespace dmcs

#endif // __RETURN_PLAN_GRAMMAR_HPP__

#include "parser/ReturnPlanGrammar.tcc"
