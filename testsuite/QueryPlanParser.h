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
  static void parseStream(std::istream& in, ContextQueryPlanMap& cqpm);
  static void parseFile(const std::string& infile, ContextQueryPlanMap& cqpm);
  static void parseString(const std::string& instr, ContextQueryPlanMap& cqpm);
};

} // namespace dmcs

#endif // __QUERY_PLAN_PARSER__INCLUDED__
