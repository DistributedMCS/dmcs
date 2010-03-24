#include "ClaspProcess.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testClasp"
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testClaspDimacs )
{
  const char* ex = getenv("EXAMPLESDIR");
  std::string dimacsfile(ex);
  dimacsfile += "/dimacs.txt";

  ClaspProcess clasp;

  clasp.addOption("0");
  clasp.addOption(dimacsfile);

  clasp.spawn();
  clasp.endoffile();

  std::istream& result = clasp.getInput();

  std::string resultfile(ex);
  resultfile += "/clasp-result.txt";
  std::fstream fresult(resultfile.c_str());

  std::string resline1;
  std::string resline2;

  while (!fresult.eof() && !result.eof())
    {
      std::getline(fresult, resline1);
      std::getline(result, resline2);

      if (!resline1.empty() && resline1[0] == 'c')
	{
          ///@todo BOOST_WARN_EQUAL does not work *grml*
          BOOST_WARN_EQUAL(resline1, resline2);
        }
      else
	{
          BOOST_CHECK_EQUAL(resline1, resline2);
        }
    }

  BOOST_CHECK(fresult.eof() && result.eof());

  // 10 ... satisfiable
  BOOST_CHECK_EQUAL(clasp.close(), 10);
}
