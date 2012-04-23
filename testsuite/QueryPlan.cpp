#include "QueryPlan.h"

namespace dmcs
{

std::ostream& ConstantCategory::print(std::ostream& os) const
{
}

std::ostream& FilterArgumentSpec::print(std::ostream& os) const
{
}

std::ostream& Filter::print(std::ostream& os) const
{
}

std::ostream& ContextQueryPlan::print(std::ostream& os) const
{
}

} // namespace dmcs

std::ostream& operator<<(std::ostream& os, const dmcs::ContextQueryPlanMap& m)
{
  using namespace dmcs;

  os << "# ContextQueryPlanMap:\n";
  os << "{\n";
  for(ContextQueryPlanMap::const_iterator it = m.begin(); it != m.end(); it++)
  {
    os << it->first << ":" << it->second << ",\n";
  }
  os << "}\n";
  return os;
}
