#ifndef __QUERY_PLAN_GRAMMAR_HPP__
#define __QUERY_PLAN_GRAMMAR_HPP__

#include <boost/spirit/include/qi.hpp>

template<typename Iterator>
struct SkipperGrammar : boost::spirit::qi::grammar<Iterator>
{
  SkipperGrammar();

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
    TargetAttribute& target = boost::fusion::at_c<0>(ctx.attributes);
    sem<Tag>()(mgr, source, target);
  }
};

#endif // __QUERY_PLAN_GRAMMAR_HPP__
