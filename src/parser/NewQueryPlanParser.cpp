#include <istream>
#include <fstream>

#include "parser/QueryPlanGrammar.hpp"
#include "parser/QueryPlanGrammar.tcc"
#include "parser/NewQueryPlanParser.hpp"

namespace dmcs {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

ContextQueryPlanMapPtr
NewQueryPlanParser::parseFile(const std::string& infile)
{
  std::ifstream ifs;

  ifs.open(infile.c_str());
  if (!ifs.is_open())
    {
      std::ostringstream oss;
      oss << "File " << infile << " not found!";
      throw std::runtime_error(oss.str());
    }
  else
    {
      return parseStream(ifs);
    }
}


ContextQueryPlanMapPtr
NewQueryPlanParser::parseStream(std::istream& in)
{
  std::ostringstream buf;
  std::string line;

  while (!in.eof())
    {
      std::getline(in, line);
      buf << line << std::endl;
      //std::cerr << "Read >>" << line << "<<" << std::endl;
    }

  if (in.fail()) in.clear();

  std::string input = buf.str();
  return parseString(input);
}


ContextQueryPlanMapPtr
NewQueryPlanParser::parseString(const std::string& str)
{
  std::string::const_iterator begIt = str.begin();
  std::string::const_iterator endIt = str.end();

  using ascii::space;
  using qi::phrase_parse;


  typedef NewSkipperGrammar<std::string::const_iterator> NewSkipper;

  NewSkipper skipper;
  QueryPlanGrammarSemantics semanticsMgr;
  NewQueryPlanGrammar<std::string::const_iterator, NewSkipper> grammar(semanticsMgr);

  bool r = phrase_parse(begIt, endIt, grammar, skipper);
   
  if(r && begIt == endIt)
  {
    std::cout << "Parsing succeeded\n";
    //std::cout << "Result is: " << std::endl;
    //std::cout << *semanticsMgr.m_QueryPlanMap << std::endl;
    return semanticsMgr.m_QueryPlanMap;
  }
  else
  {
    std::cout << "Parsing Failed" << std::endl;
    throw std::runtime_error("Query plan parsing failed");
    }
}


} // namespace dmcs
