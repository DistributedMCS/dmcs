#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <istream>
#include <string>
#include <boost/shared_ptr.hpp>

#include "parser/BaseParser.hpp"
#include "parser/QueryPlanGrammar.hpp"

namespace dmcs {

template<typename GrammarType, typename SemanticsType, typename ReturnType>
class Parser
{
public:
  static boost::shared_ptr<ReturnType> parseStream(std::istream& in);
  static boost::shared_ptr<ReturnType> parseFile(const std::string& infile);
  static boost::shared_ptr<ReturnType> parseString(const std::string& instr);
};

typedef NewSkipperGrammar<std::string::const_iterator> NewSkipper;
typedef Parser<NewQueryPlanGrammar<std::string::const_iterator, NewSkipper>, QueryPlanGrammarSemantics, ContextQueryPlanMap> QueryPlanParser_t;

} // namespace dmcs

#endif // __PARSER_HPP__
