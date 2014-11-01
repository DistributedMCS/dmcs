#ifndef __PARSER_TCC__
#define __PARSER_TCC__

#include <istream>
#include <fstream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/spirit/include/qi.hpp>

#include "parser/BaseParser.hpp"

namespace dmcs {

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template<typename GrammarType, typename SemanticsType, typename ReturnType>
boost::shared_ptr<ReturnType>
Parser<GrammarType, SemanticsType, ReturnType>::parseFile(const std::string& infile)
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



template<typename GrammarType, typename SemanticsType, typename ReturnType>
boost::shared_ptr<ReturnType>
Parser<GrammarType, SemanticsType, ReturnType>::parseStream(std::istream& in)
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


template<typename GrammarType, typename SemanticsType, typename ReturnType>
boost::shared_ptr<ReturnType>
Parser<GrammarType, SemanticsType, ReturnType>::parseString(const std::string& str)
{
  std::string::const_iterator begIt = str.begin();
  std::string::const_iterator endIt = str.end();

  using ascii::space;
  using qi::phrase_parse;


  typedef NewSkipperGrammar<std::string::const_iterator> NewSkipper;

  NewSkipper skipper;
  SemanticsType semanticsMgr;
  //GrammarType<std::string::const_iterator, NewSkipper> grammar(semanticsMgr);
  GrammarType grammar(semanticsMgr);

  bool r = phrase_parse(begIt, endIt, grammar, skipper);
   
  if(r && begIt == endIt)
  {
    std::cout << "Parsing succeeded\n";
    //std::cout << "Result is: " << std::endl;
    //std::cout << *semanticsMgr.m_QueryPlanMap << std::endl;
    return semanticsMgr.m_ParsedResult;
  }
  else
  {
    std::cout << "Parsing Failed" << std::endl;
    throw std::runtime_error("Query plan parsing failed");
  }
}

} // namespace dmcs

#endif // __PARSER_TCC__
