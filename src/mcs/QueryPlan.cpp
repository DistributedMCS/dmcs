#include <algorithm>
#include <iostream>

#include "mcs/QueryPlan.h"

namespace dmcs
{

std::ostream&
operator<<(std::ostream &os, const ConstantList &l)
{
  std::copy(l.begin(), l.end(), std::ostream_iterator<std::string>(os, ", "));
  return os;
}

std::ostream& ConstantCategory::print(std::ostream& os) const
{
  os << "      {Category: " << name << ", Constants: [";
  std::copy(constants.begin(), constants.end(), std::ostream_iterator<std::string>(os, ", "));
  os << "]}";

  return os;
}

std::ostream&
operator<<(std::ostream &os, const ConstantCategoryList &l)
{
  std::copy(l.begin(), l.end(), std::ostream_iterator<ConstantCategory>(os, "\n"));
  return os;
}


std::ostream& FilterArgumentSpec::print(std::ostream& os) const
{
}

std::ostream& Filter::print(std::ostream& os) const
{
}

std::ostream& ContextQueryPlan::print(std::ostream& os) const
{
  os << "  {\n";
  os << "    ContextID: " << ctx << "," << std::endl;
  if( !!constants && !constants->empty() )
    os << "    Constants: [" << *constants << "]," << std::endl;
  if( !!constCats && !constCats->empty() )
    os << "    ConstantCategories:" << std::endl
       << "    [" << std::endl
       << *constCats
       << "    ]" << std::endl;
  if( !!preds && !preds->empty() )
      os << "    TODO preds,\n";
  if( !!filters && !filters->empty() )
      os << "    TODO filters,\n";
  if( !!localSignature && !localSignature->empty() )
  {
      os << "    LocalSignature:\n";
      os << "    [\n";
      os << *localSignature;
      os << "    ],\n";
  }
  if( !!groundInputSignature && !groundInputSignature->empty() )
  {
      os << "    InputSignature:\n";
      os << "    [\n";
      os << *groundInputSignature;
      os << "    ],\n";
  }
  if( !!outputProjections && !outputProjections->empty() )
      os << "    TODO outputProjections,\n";
  os << "  }\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const dmcs::ContextQueryPlanMap& m)
{
  using namespace dmcs;

  os << "# ContextQueryPlanMap:\n";
  os << "[\n";
  for(ContextQueryPlanMap::const_iterator it = m.begin(); it != m.end(); it++)
  {
    os << it->second << ",\n";
  }
  os << "]\n";
  return os;
}


} // namespace dmcs

