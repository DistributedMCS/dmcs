#include "Parser.hpp"

#include "QueryPlan.h"
#include "QueryPlanParser.h"

#include <iostream>
#include <fstream>

#if 0 
void readQueryPlan (const std::string& str)
{

//  std::cout << str << std::endl;

  std::string::const_iterator begIt = str.begin();
  std::string::const_iterator endIt = str.end();

  using ascii::space;
  using qi::phrase_parse;

  QueryPlanGrammar<std::string::const_iterator> queryPlanGrammar;
  
  NonGroundSignatureList ngsList;
  
  bool r = phrase_parse(begIt, endIt, queryPlanGrammar, space, ngsList);
   
  if(r && begIt == endIt)
  {
    std::cout << "Parsing succeeded\n";
    std::cout << "Result is :\n";
    NonGroundSignatureListPtr ngsListPtr (new NonGroundSignatureList(ngsList));
    std::cout << ngsListPtr;
 }
  else
  {
    std::cout << "Parsing Failed\n";
  }

}
#endif

void 
testParser (const std::string& fileName)
{
  std::cout << "File name: " << fileName << std::endl;
  std::ifstream inp;
  inp.open(fileName.c_str());

  if(!inp.is_open())
  {
    std::cout << "File cannot be opened.\n";
  }
 
  inp.seekg(0, std::ios::end);
  int length = inp.tellg();
  inp.seekg(0, std::ios::beg);
  

  std::string buffer;
  buffer.resize(length);
  inp.read(&buffer[0], length);
  inp.close(); 

  std::cerr << "Following query plan will be parsed:\n";
  std::cerr << buffer << "\n";
  
  dmcs::ContextQueryPlanMapPtr plan =
    dmcs::QueryPlanParser::parseString(buffer);

  std::cerr << "Got result:\n";
  std::cerr << *plan << "\n";
}

int main(int argc, char** argv)
{
  std::cout << "Hello dmcs query plan parser\n";  

  //std::string fileNames [] = {"queryPlanParserTest1.txt","queryPlanParserTest2.txt","queryPlanParserTest3.txt","queryPlanParserTest4.txt"};
  std::string fileNames [] = {"queryPlanParserTest1.txt"};
  int index = 1;
  for(int i = 0; i < index; i++)
  {
    testParser(fileNames[i]);
  }

 // delete [] fileNames;  
  return 0;
}
