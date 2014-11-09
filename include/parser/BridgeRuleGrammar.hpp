#ifndef __BRIGDE_RULE_GRAMMAR_HPP__
#define __BRIGDE_RULE_GRAMMAR_HPP__

#include "mcs/QueryPlan.h"
#include "mcs/BridgeRuleTable.h"
#include "mcs/NewNeighbor.h"

namespace dmcs {

typedef std::pair<BridgeRuleTablePtr, NewNeighborVecPtr> BridgeRuleParserReturnVal;

struct ContextQueryPlanPtr_CtxID
{
  ContextQueryPlanPtr m_QueryPlan;
  const std::size_t   m_CtxID;

  ContextQueryPlanPtr_CtxID(const ContextQueryPlanPtr &qp,
			    const std::size_t ctx_id)
    : m_QueryPlan(qp),
      m_CtxID(ctx_id)
  { }
};

class BridgeRuleGrammarSemantics
{
public:
  ContextQueryPlanPtr   m_QueryPlan;
  const std::size_t     m_CtxID;

  BridgeRuleTablePtr    m_BridgeRules;
  NewNeighborVecPtr     m_Neighbors;
  std::size_t           m_NeighborOffset;
  std::set<std::size_t> m_NeighborIDs;

public:
  BridgeRuleGrammarSemantics(const ContextQueryPlanPtr_CtxID &inp)
    : m_QueryPlan(inp.m_QueryPlan),
      m_CtxID(inp.m_CtxID),
      m_BridgeRules(new BridgeRuleTable),
      m_Neighbors(new NewNeighborVec),
      m_NeighborOffset(0)
  { }

  #define DMCS_DEFINE_SEMANTIC_ACTION(name, targettype) \
    struct name: \
      SemanticActionBase<BridgeRuleGrammarSemantics, targettype, name> \
    { \
      name(BridgeRuleGrammarSemantics& mgr): name ::base_type(mgr) {} \
    };

  DMCS_DEFINE_SEMANTIC_ACTION(passPredToBelief, std::string);
  DMCS_DEFINE_SEMANTIC_ACTION(passIdentToBelief, std::string);
  DMCS_DEFINE_SEMANTIC_ACTION(getBridgeAtom, ID);
  DMCS_DEFINE_SEMANTIC_ACTION(getBridgeLiteral, ID);
  DMCS_DEFINE_SEMANTIC_ACTION(getBridgeRule, boost::spirit::unused_type);
  #undef DMCS_DEFINE_SEMANTIC_ACTION
};

//! basic BridgeRuleGrammar
template<typename Iterator, typename NewSkipper>
struct BridgeRuleGrammarBase
{
  BridgeRuleGrammarSemantics &sem;

  BridgeRuleGrammarBase(BridgeRuleGrammarSemantics&);

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
  typename Rule<>::type start;
  typename Rule<std::string>::type ident, belief;
  typename Rule<ID>::type bridgeAtom, bridgeLiteral, bridgeRule;
  typename Rule<std::vector<string> >::type terms;
  typename Rule<fusion::vector2<std::string, std::vector<std::string> > >::type predicate;
};


template<typename Iterator, typename NewSkipper>
struct NewBridgeRuleGrammar : 
  BridgeRuleGrammarBase<Iterator, NewSkipper>,
  boost::spirit::qi::grammar<Iterator, NewSkipper>
{
  typedef BridgeRuleGrammarBase<Iterator, NewSkipper> GrammarBase;
  typedef boost::spirit::qi::grammar<Iterator, NewSkipper> QiBase;

  NewBridgeRuleGrammar(BridgeRuleGrammarSemantics& sem)
    : GrammarBase(sem),
      QiBase(GrammarBase::start)
  { }      
};

} // namespace dmcs

#include "parser/BridgeRuleGrammar.tcc"

#endif // __BRIGDE_RULE_GRAMMAR_HPP__

// Local Variables:
// mode: C++
// End:
