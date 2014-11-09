#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <istream>
#include <string>
#include <boost/shared_ptr.hpp>

#include "mcs/BeliefTable.h"
#include "parser/BaseParser.hpp"
#include "parser/BridgeRuleGrammar.hpp"
#include "parser/DLVResultGrammar.hpp"
#include "parser/QueryPlanGrammar.hpp"
#include "parser/ReturnPlanGrammar.hpp"

namespace dmcs {

template<typename GrammarType, typename SemanticsType, typename ReturnType>
class Parser
{
public:
  Parser()
  { }

  Parser(const SemanticsType &semanticsMgr)
    : m_SemanticsMgr(semanticsMgr)
  { }

  ReturnType parseStream(std::istream& in);
  ReturnType parseFile(const std::string& infile);
  ReturnType parseString(const std::string& instr);

protected:
  SemanticsType m_SemanticsMgr;
};


template<typename InputType, typename GrammarType, typename SemanticsType, typename ReturnType>
class ParserWithInput : public Parser<GrammarType, SemanticsType, ReturnType>
{
public:
  ParserWithInput(const InputType &inp)
    : Parser<GrammarType,SemanticsType,ReturnType>(SemanticsType(inp))
  { }
};




typedef NewSkipperGrammar<std::string::const_iterator> NewSkipper;
typedef Parser<NewQueryPlanGrammar<std::string::const_iterator, NewSkipper>, QueryPlanGrammarSemantics, ContextQueryPlanMapPtr> QueryPlanParser_t;
typedef Parser<NewReturnPlanGrammar<std::string::const_iterator, NewSkipper>, ReturnPlanGrammarSemantics, ReturnPlanMapPtr> ReturnPlanParser_t;
typedef ParserWithInput<BeliefTablePtr_CtxID, NewDLVResultGrammar<std::string::const_iterator, NewSkipper>, DLVResultGrammarSemantics, NewBeliefState*> DLVResultParser_t;
typedef ParserWithInput<ContextQueryPlanMapPtr_CtxID, NewBridgeRuleGrammar<std::string::const_iterator, NewSkipper>, BridgeRuleGrammarSemantics, BridgeRuleParserReturnVal> BridgeRuleParser_t;
} // namespace dmcs

#endif // __PARSER_HPP__

#include "parser/Parser.tcc"
