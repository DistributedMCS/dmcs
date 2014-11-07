#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <istream>
#include <string>
#include <boost/shared_ptr.hpp>

#include "mcs/BeliefTable.h"
#include "parser/BaseParser.hpp"
#include "parser/QueryPlanGrammar.hpp"
#include "parser/ReturnPlanGrammar.hpp"

namespace dmcs {

template<typename InputType=void, typename GrammarType=void, typename SemanticsType=void, typename ReturnType=void>
class Parser
{
public:
  static ReturnType parseStream(InputType &inp, std::istream& in);
  static ReturnType parseFile(InputType &inp, const std::string& infile);
  static ReturnType parseString(InputType &inp, const std::string& instr);
};


template<typename GrammarType, typename SemanticsType, typename ReturnType>
class Parser<void, GrammarType, SemanticsType, ReturnType>
{
public:
  static ReturnType parseStream(std::istream& in);
  static ReturnType parseFile(const std::string& infile);
  static ReturnType parseString(const std::string& instr);
};


typedef NewSkipperGrammar<std::string::const_iterator> NewSkipper;
typedef Parser<void, NewQueryPlanGrammar<std::string::const_iterator, NewSkipper>, QueryPlanGrammarSemantics, ContextQueryPlanMapPtr> QueryPlanParser_t;
typedef Parser<void, NewReturnPlanGrammar<std::string::const_iterator, NewSkipper>, ReturnPlanGrammarSemantics, ReturnPlanMapPtr> ReturnPlanParser_t;
typedef Parser<BeliefTablePtr, NewReturnPlanGrammar<std::string::const_iterator, NewSkipper>, ReturnPlanGrammarSemantics, NewBeliefState*> DLVResultParser_t;
} // namespace dmcs

#endif // __PARSER_HPP__

#include "parser/Parser.tcc"
