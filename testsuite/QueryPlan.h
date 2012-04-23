#ifndef __QUERY_PLAN__INCLUDED__
#define __QUERY_PLAN__INCLUDED__

#include "mcs/BeliefState.h"
#include "mcs/BeliefTable.h"
#include "mcs/Printhelpers.h"

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#include <list>
#include <map>
#include <iostream>

namespace dmcs
{

typedef uint32_t ContextID;
typedef std::list<std::string>
  ConstantList;
typedef std::map<ContextID, BeliefState>
  OutputProjectionMap;
typedef std::map<std::string, unsigned>
  PredicateArityMap;

struct ConstantCategory:
  private ostream_printable<ConstantCategory>
{
  // name of the category
  std::string name;
  // constants in the category
  ConstantList constants;

  ConstantCategory(
      const std::string& name):
    name(name) {}

  std::ostream& print(std::ostream& os) const;
};
typedef std::list<ConstantCategory>
  ConstantCategoryList;

struct FilterArgumentSpec:
  private ostream_printable<FilterArgumentSpec>
{
  std::list<std::string> usingCategories;

  std::ostream& print(std::ostream& os) const;
};

typedef std::map<unsigned, FilterArgumentSpec>
  FilterArgumentSpecMap;

struct Filter:
  private ostream_printable<Filter>
{
  std::string name;
  std::string predicate;
  FilterArgumentSpecMap arguments;

  Filter(const std::string& name, const std::string& predicate):
    name(name), predicate(predicate) {}

  std::ostream& print(std::ostream& os) const;
};
typedef std::list<Filter>
  FilterList;

struct ContextQueryPlan:
  private ostream_printable<ContextQueryPlan>
{
  //id of context
  ContextID ctxID;

  // below: information before grounding and/or optimization

protected:
  // local constants
  ConstantList constants;

  // categories of local constants (without implicit categories)
  ConstantCategoryList constCats;

public:
  // throws if we try to add a constant that is constant category name
  void addConstant();
  // throws if we try to add a category that has as name a constant
  void addConstantCategory();

  // retrieves constants into category, maybe single constant via implicit category
  void getConstantsForCategory(ConstantList& container) const;

public:
  // predicates
  PredicateArityMap preds;

  // filters
  FilterList filters;

  // full signature of context
  // (only known for own context)
  BeliefTable localSignature;

  // below: information for evaluation

  // for which bits do we know the predicates?
  // this is necessary for evaluating bridge rules given bitsets
  // (known for all contexts where this context requests information from)
  BeliefTable groundInputSignature;

  // for each other context, which bits do we send to this context?
  // this is necessary for returning bitset results
  // (only known for own context)
  OutputProjectionMap outputProjections;

  std::ostream& print(std::ostream& os) const;
};

typedef std::map<ContextID, ContextQueryPlan>
  ContextQueryPlanMap;

} //namespace dmcs

std::ostream& operator<< (std::ostream& os, const dmcs::ContextQueryPlanMap& m);

#if 0
inline std::ostream&
operator<< (std::ostream& os, const Category& c)
{
  return  os << "[ Category: " << c.catSymbol << ", Constants: [" << c.constantListPtr << "] ] ";
}	

std::ostream&
operator<< (std::ostream& os, const CategoryPtr& p)
{
  return os << *p;
}	


std::ostream&
operator<< (std::ostream& os, const CategoryList& c)
{
  for(CategoryList::const_iterator it = c.begin(); it != c.end(); it++)
  {
    os << *it << " ";
  }
  
  return os;
}

std::ostream&
operator<< (std::ostream& os, const CategoryListPtr& p)
{
  return os << *p;
}	


std::ostream&
operator<< (std::ostream& os, const FilterArgument& f)
{
  return os << "[ Position: " << f.first << ", " << "Using: [" << f.second << "] ]";
}
		
std::ostream&
operator<< (std::ostream& os, const FilterArgumentPtr& p)
{
  return os << *p;
}

std::ostream&
operator<< (std::ostream& os, const FilterArgumentList& f)
{
  for(FilterArgumentList::const_iterator it = f.begin(); it != f.end(); it++)
  {
    os << *it << " ";
  }

  return os;
}

std::ostream&
operator<< (std::ostream& os, const FilterArgumentListPtr& p)
{
  return os << *p;
}


inline std::ostream&
operator<< (std::ostream& os, const Filter& f)
{
  return os << "[ Name: " << f.filterName << ", Pred: " << f.predSymbol << ", Arguments: [ " << f.filterArgumentListPtr << " ]";
}

std::ostream&
operator<< (std::ostream& os, const FilterPtr& p)
{
  return os << *p;
}

std::ostream&
operator<< (std::ostream& os, const FilterList& f)
{
  for(FilterList::const_iterator it = f.begin(); it != f.end(); it++)
  {
    os << *it << " ";
  }

  return os;
}

std::ostream&
operator<< (std::ostream& os, const FilterListPtr& p)
{
  return os << *p;
}

#endif

#endif // __QUERY_PLAN__INCLUDED__
