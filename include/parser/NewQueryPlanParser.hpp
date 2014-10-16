#ifndef __NEW_QUERY_PLAN_PARSER_HPP__
#define __NEW_QUERY_PLAN_PARSER_HPP__

#include <istream>
#include <string>

#include "mcs/QueryPlan.h"

namespace dmcs {

class NewQueryPlanParser
{
public:
  static ContextQueryPlanMapPtr parseStream(std::istream& in);
  static ContextQueryPlanMapPtr parseFile(const std::string& infile);
  static ContextQueryPlanMapPtr parseString(const std::string& instr);
};

#endif // __NEW_QUERY_PLAN_PARSER_HPP__

} // namespace dmcs
