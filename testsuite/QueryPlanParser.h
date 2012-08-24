#ifndef __QUERY_PLAN_PARSER__INCLUDED__
#define __QUERY_PLAN_PARSER__INCLUDED__

#include "QueryPlan.h"

#include <istream>
#include <string>

namespace dmcs
{

class QueryPlanParser
{
public:
  static ContextQueryPlanMapPtr parseStream(std::istream& in);
  static ContextQueryPlanMapPtr parseFile(const std::string& infile);
  static ContextQueryPlanMapPtr parseString(const std::string& instr);
};

} // namespace dmcs

#endif // __QUERY_PLAN_PARSER__INCLUDED__
