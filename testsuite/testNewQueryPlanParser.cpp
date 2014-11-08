#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testNewQueryPlanParser"
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

#include "mcs/QueryPlan.h"
#include "parser/QueryPlanGrammar.hpp"
#include "parser/QueryPlanGrammar.tcc"
#include "parser/Parser.hpp"
#include "parser/Parser.tcc"

using namespace dmcs;

void 
testParser(const std::string& fileName)
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

  QueryPlanParser_t queryplan_parser;
  ContextQueryPlanMapPtr plan = queryplan_parser.parseString(buffer);

  std::cerr << "Got result:\n";
  //std::cerr << *plan << "\n";
}


BOOST_AUTO_TEST_CASE ( testNewQueryPlanParser )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string fileNames [] = {"queryPlanParserTest0.txt"};
  int index = 1;
  for(int i = 0; i < index; i++)
  {
    std::string filename(ex);
    filename = filename + "/" + fileNames[i];

    testParser(filename);
  }
}

// Local Variables:
// mode: C++
// End:
