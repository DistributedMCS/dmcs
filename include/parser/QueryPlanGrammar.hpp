#ifndef __QUERY_PLAN_GRAMMAR_HPP__
#define __QUERY_PLAN_GRAMMAR_HPP__

#include <list>
#include <boost/spirit/include/qi.hpp>

#include "mcs/QueryPlan.h"

namespace dmcs {

namespace fusion = boost::fusion;

template<typename Iterator>
struct NewSkipperGrammar : boost::spirit::qi::grammar<Iterator>
{
  NewSkipperGrammar();

  boost::spirit::qi::rule<Iterator> ws;
};


// generic semantic action processor which creates useful compile-time error messages
// (if the grammar compiles, this class is never used, this means we have a handler for each action)
template<typename Tag>
struct sem
{
  template<typename Mgr, typename Source, typename Target>
  void operator()(Mgr& mgr, const Source& source, Target& target)
  { BOOST_MPL_ASSERT(( boost::is_same< Source, void > )); }
};


// base class for semantic actions
// this class delegates to sem<Tag>::operator() where all the true processing happens (hidden in compilation unit)
template<typename ManagerClass, typename TargetAttribute, typename Tag>
struct SemanticActionBase
{
  typedef SemanticActionBase<ManagerClass, TargetAttribute, Tag> base_type;

  ManagerClass& mgr;
  SemanticActionBase(ManagerClass& mgr): mgr(mgr) {}

  template<typename SourceAttributes, typename Ctx>
  void operator()(const SourceAttributes& source, Ctx& ctx, boost::spirit::qi::unused_type) const
  {
    TargetAttribute& target = fusion::at_c<0>(ctx.attributes);
    sem<Tag>()(mgr, source, target);
  }
};


class QueryPlanGrammarSemantics
{
public:
  ContextQueryPlanMapPtr m_QueryPlanMap;
  ContextQueryPlanPtr    m_CurrentQueryPlan;
  ConstantListPtr        m_CurrentConstantList;

public:
  QueryPlanGrammarSemantics()
  { }

  #define DMCS_DEFINE_SEMANTIC_ACTION(name, targettype) \
    struct name: \
      SemanticActionBase<QueryPlanGrammarSemantics, targettype, name> \
    { \
      name(QueryPlanGrammarSemantics& mgr): name ::base_type(mgr) {} \
    };

  DMCS_DEFINE_SEMANTIC_ACTION(setContextID, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(setHostName, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(setPort, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(getCurrentConstantList, ConstantListPtr);
  DMCS_DEFINE_SEMANTIC_ACTION(setConstantList, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(setConstantCategories, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(setPredicates, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(seekConstantCategory, ConstantListPtr);
  DMCS_DEFINE_SEMANTIC_ACTION(setFilters, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(setLocalSignature, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(setInputSignature, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(insertIntoMap, const boost::spirit::unused_type);
  DMCS_DEFINE_SEMANTIC_ACTION(registerAndInsertIntoBeliefSet, BeliefTablePtr);

  #undef DMCS_DEFINE_SEMANTIC_ACTION
};


//! basic QueryPlanGrammar
template<typename Iterator, typename NewSkipper>
struct QueryPlanGrammarBase
{
  QueryPlanGrammarSemantics &sem;
  QueryPlanGrammarBase(QueryPlanGrammarSemantics&);

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
  typename Rule<>::type start, contextQueryPlan;
  typename Rule<std::string>::type ident, hostName, catSymbol, predSymbol, filterName;
  typename Rule<ConstantListPtr>::type constants;
  typename Rule<std::vector<fusion::vector2<std::string, ConstantListPtr > > >::type constantCategories;
  typename Rule<fusion::vector2<std::string, ConstantListPtr > >::type category;
  typename Rule<std::vector<fusion::vector2<std::string, std::size_t > > >::type predicates;
  typename Rule<fusion::vector2<std::string, std::size_t > >::type predicate;
  typename Rule<std::vector<fusion::vector3<std::string, std::string, std::vector<fusion::vector2<std::size_t, ConstantListPtr> > > > >::type filters;
  typename Rule<fusion::vector3<std::string, std::string, std::vector<fusion::vector2<std::size_t, ConstantListPtr> > > >::type filter;
  typename Rule<std::vector<fusion::vector2<std::size_t, ConstantListPtr> > >::type arguments;
  typename Rule<fusion::vector2<std::size_t, ConstantListPtr> >::type argument;
  typename Rule<ConstantListPtr>::type useCategory;
  typename Rule<BeliefTablePtr>::type signature;
  typename Rule<fusion::vector2<IDAddress, std::vector<std::string> > >::type id_with_ground_tuple;
};


template<typename Iterator, typename NewSkipper>
struct NewQueryPlanGrammar : 
  QueryPlanGrammarBase<Iterator, NewSkipper>,
  boost::spirit::qi::grammar<Iterator, NewSkipper>
{
  typedef QueryPlanGrammarBase<Iterator, NewSkipper> GrammarBase;
  typedef boost::spirit::qi::grammar<Iterator, NewSkipper> QiBase;

  NewQueryPlanGrammar(QueryPlanGrammarSemantics& sem)
    : GrammarBase(sem),
      QiBase(GrammarBase::start)
  { }      
};

} // namespace dmcs

#endif // __QUERY_PLAN_GRAMMAR_HPP__
