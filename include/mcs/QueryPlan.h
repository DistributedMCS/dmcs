#ifndef __QUERY_PLAN__INCLUDED__
#define __QUERY_PLAN__INCLUDED__

#include "mcs/ID.h"
#include "mcs/NewBeliefState.h"
#include "mcs/BeliefTable.h"
#include "mcs/Printhelpers.h"

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#include <list>
#include <map>
#include <iostream>

namespace dmcs
{

typedef std::size_t ContextID;
typedef std::list<std::string> ConstantList;
typedef boost::shared_ptr<ConstantList> ConstantListPtr;
typedef std::map<ContextID, NewBeliefState> OutputProjectionMap;
typedef std::map<std::string, std::size_t> PredicateArityMap;

struct ConstantCategory : private ostream_printable<ConstantCategory>
{
  // name of the category
  std::string name;

  // constants in the category
  ConstantListPtr constants;

  ConstantCategory(const std::string& name)
    : name(name) 
  { }

  std::ostream& print(std::ostream& os) const;
};
typedef std::list<ConstantCategory> ConstantCategoryList;

typedef std::map<std::size_t, ConstantListPtr> FilterArgumentSpecMap;

struct Filter : private ostream_printable<Filter>
{
  std::string name;
  std::string predicate;
  FilterArgumentSpecMap arguments;

  Filter(const std::string& name, const std::string& predicate)
    : name(name), predicate(predicate) 
 { }

  std::ostream& print(std::ostream& os) const;
};
typedef std::list<Filter> FilterList;


typedef boost::shared_ptr<ConstantCategoryList> ConstantCategoryListPtr;
typedef boost::shared_ptr<PredicateArityMap> PredicateArityMapPtr;
typedef boost::shared_ptr<FilterList> FilterListPtr;
typedef boost::shared_ptr<BeliefTable> BeliefTablePtr;
typedef boost::shared_ptr<OutputProjectionMap> OutputProjectionMapPtr;

struct ContextQueryPlan : private ostream_printable<ContextQueryPlan>
{
  // id of context
  ContextID ctx;

  // host name and port of the context
  std::string hostname;
  int port;

  // below: information before grounding and/or optimization

  // local constants
  ConstantListPtr constants;

  // categories of local constants (without implicit categories)
  ConstantCategoryListPtr constCats;

  // predicates
  PredicateArityMapPtr preds;

  // filters
  FilterListPtr filters;

  // full signature of context
  // (only known for own context)
  BeliefTablePtr localSignature;

  // below: information for evaluation

  // for which bits do we know the predicates?
  // this is necessary for evaluating bridge rules given bitsets
  // (known for all contexts where this context requests information from)
  BeliefTablePtr groundInputSignature;

  // for each other context, which bits do we send to this context?
  // this is necessary for returning bitset results
  // (only known for own context)
  OutputProjectionMapPtr outputProjections;

  ContextQueryPlan() 
  { }


  ContextQueryPlan(ContextID ctx, const std::string& hostname, int port,
		  ConstantListPtr constants,
		  ConstantCategoryListPtr constCats, PredicateArityMapPtr preds,
		  FilterListPtr filters, BeliefTablePtr localSignature,
		  BeliefTablePtr groundInputSignature, OutputProjectionMapPtr outputProjections)
    : ctx(ctx), hostname(hostname), port(port), constants(constants),
      constCats(constCats), preds(preds),
      filters(filters), localSignature(localSignature),
      groundInputSignature(groundInputSignature), 
      outputProjections(outputProjections) 
  { }

  std::ostream& print(std::ostream& os) const;
};

typedef boost::shared_ptr<ContextQueryPlan> ContextQueryPlanPtr;
typedef std::map<ContextID, ContextQueryPlan> ContextQueryPlanMap;

typedef boost::shared_ptr<ContextQueryPlanMap>
  ContextQueryPlanMapPtr;

typedef boost::shared_ptr<const ContextQueryPlanMap>
  ContextQueryPlanMapConstPtr;

std::ostream& operator<< (std::ostream& os, const ConstantList &l);
std::ostream& operator<< (std::ostream& os, const ConstantCategoryList &l);
std::ostream& operator<< (std::ostream& os, const ContextQueryPlanMap &m);
std::ostream& operator<< (std::ostream& os, const PredicateArityMap &m);
std::ostream& operator<< (std::ostream& os, const FilterList &l);

} //namespace dmcs


#endif // __QUERY_PLAN__INCLUDED__

// Local Variables:
// mode: C++
// End:
