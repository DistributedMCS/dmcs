#ifndef __DLV_RESULT_GRAMMAR_HPP__
#define __DLV_RESULT_GRAMMAR_HPP__

#include "mcs/NewBeliefState.h"
#include "mcs/BeliefTable.h"

namespace dmcs {

class DLVResultGrammarSemantics
{
public:
  BeliefTablePtr     m_BTab;
  const std::size_t  m_CtxID;
  NewBeliefState    *m_ParsedResult;

public:
  DLVResultGrammarSemantics(const BeliefTablePtr &btab)
    : m_BTab(btab)
  { }

  #define DMCS_DEFINE_SEMANTIC_ACTION(name, targettype) \
    struct name: \
      SemanticActionBase<DLVResultGrammarSemantics, targettype, name> \
    { \
      name(DLVResultGrammarSemantics& mgr): name ::base_type(mgr) {} \
    };

  DMCS_DEFINE_SEMANTIC_ACTION(handleLiteral, boost::spirit::unused_type);

  #undefine DMCS_DEFINE_SEMANTIC_ACTION
};


//! basic DLVResultGrammar
template<typename Iterator, typename NewSkipper>
struct DLVResultGrammarBase
{
  DLVResultGrammarSemantics &sem;

  DLVResultGrammarBase(DLVResultGrammarSemantics&);

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
  typename Rule<>::type            dlvline, costline, answerset, mlit, flit;
  typename Rule<std::string>::type ident, fact, number;
};


template<typename Iterator, typename NewSkipper>
struct NewDLVResultGrammar : 
  DLVResultGrammarBase<Iterator, NewSkipper>,
  boost::spirit::qi::grammar<Iterator, NewSkipper>
{
  typedef DLVResultGrammarBase<Iterator, NewSkipper> GrammarBase;
  typedef boost::spirit::qi::grammar<Iterator, NewSkipper> QiBase;

  NewDLVResultGrammar(DLVResultGrammarSemantics& sem)
    : GrammarBase(sem),
      QiBase(GrammarBase::start)
  { }      
};

} // namespace dmcs

#include "parser/DLVResultGrammar.tcc"

#endif // __DLV_RESULT_GRAMMAR_HPP__
