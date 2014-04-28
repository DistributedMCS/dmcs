#include "dmcs/Log.h"
#include "relsat-20070104/SATInstance.h"
#include "relsat-20070104/SATSolver.h"



#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSATModelOrder"
#include <boost/test/unit_test.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace dmcs;


void
run_solver(TheoryPtr theory, std::size_t no_atoms, const std::string& output_file)
{
  std::ofstream file;
  file.open(output_file.c_str());
  SATInstance* xInstance = new SATInstance(file);
  SATSolver* xSATSolver = new SATSolver(xInstance, file, NULL);

  xInstance->readTheory(theory, no_atoms);

  relsat_enum eResult;
  std::size_t models_sofar = 0;
  long int solutions_limit = 0;
  eResult = xSATSolver->eSolve(solutions_limit, models_sofar);
  file.close();
}



void
compare(const std::string& filename1, const std::string& filename2)
{
  BOOST_TEST_MESSAGE("Comparing " << filename1 << " and " << filename2);

  std::ifstream file1(filename1.c_str());
  std::ifstream file2(filename2.c_str());

  assert (file1.is_open() && file2.is_open());

  std::string line1;
  std::string line2;
  while (!file1.eof() && !file2.eof())
    {
      std::getline(file1, line1);
      std::getline(file2, line2);
      BOOST_CHECK_EQUAL(line1, line2);
    }
  BOOST_CHECK(file1.eof() && file2.eof());
}


BOOST_AUTO_TEST_CASE ( testSATModelOrder )
{
  init_loggers("testSATModelOrder");

  // Read a dummy theory from a text file
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string inputfile(ex);
  inputfile += "/input.txt";

  std::string line;
  std::ifstream input(inputfile.c_str());

  assert (input.is_open());

  std::getline(input, line);
  boost::tokenizer<> tok(line);
  
  // must be of the form "p cnf NoAtoms NoClauses"
  assert (std::distance(tok.begin(), tok.end()) == 4);
  boost::tokenizer<>::const_iterator na_it = tok.begin();
  boost::tokenizer<>::const_iterator nc_it = tok.begin();
  std::advance(na_it, 2);
  std::advance(nc_it, 3);
  
  std::size_t no_atoms = std::atoi(na_it->c_str());
  std::size_t no_clauses = std::atoi(nc_it->c_str());
  
  TheoryPtr theory(new Theory);
  for (std::size_t i = 0; i < no_clauses; ++i)
    {
      int atom;
      ClausePtr cl(new dmcs::Clause);
      input >> atom;
      while (atom != 0)
	{
	  cl->push_back(atom);
	  input >> atom;
	}
      theory->push_back(cl);
    }

  input.close();

  // Run RelSat solver many times and compare the results between 2 consecutive runs
  const std::size_t MAX_TEST = 2;
  std::string previous_output(ex);
  previous_output += "/fixed_output.txt";

  for (char i = 'a'; i < 'a' + MAX_TEST; ++i)
    {
      std::string output_file = "output";
      output_file += i;
      std::cerr << "Running solver, send results to: " << output_file << std::endl;
      run_solver(theory, no_atoms, output_file);

      // now compare this output and the previous one
      // first time (i = 'a'), we will compare with a fixed output
      compare(output_file, previous_output);

      previous_output = output_file;
    }
}
