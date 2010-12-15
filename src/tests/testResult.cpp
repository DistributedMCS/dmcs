#include <iostream>
#include <sstream>
#include <string>
#include <boost/tokenizer.hpp>

#include "ClaspProcess.h"
#include "DimacsVisitor.h"
#include "Theory.h"

#include "parser/CnfBuilder.h"
#include "parser/CnfGrammar.h"
#include "parser/ParserDirector.h"

using namespace dmcs;
using namespace dmcs::generator;

TheoryPtr theory(new Theory);
BiMapId2Name id2name;
BiMapId2Lid id2lid;
std::ostringstream global_sat_stream;
int total_answers;
int pass;

void
create_local_id()
{
  int i = 1;
  for (BiMapId2Name::left_const_iterator it = id2name.left.begin(); 
       it != id2name.left.end(); ++i, ++it)
    {
      BiMapId2Lid::value_type p(it->first, i);
      id2lid.insert(p);
    }
}

void
print_name_ids()
{
  for (BiMapId2Name::left_const_iterator it = id2name.left.begin(); it != id2name.left.end(); ++it)
    {
      std::cerr << it->second << " --> " << it->first << " --> " << id2lid.left.at(it->first) << std::endl;
    }
}

void
check_answer(const std::string& line)
{
  boost::char_separator<char> sep(", ");
  boost::tokenizer<boost::char_separator<char> > answers(line, sep);
  int count = 0;
  std::ostringstream answer_stream;

#ifdef DEBUG
  std::cerr << "Processing: \"" << line << "\"" << std::endl;
  std::cerr << "Which is equal to: " << std::endl;
  for (boost::tokenizer<boost::char_separator<char> >::iterator it = answers.begin(); 
       it != answers.end(); ++it)
    {
      if (*it != " ")
	{
	  std::string atom;

	  if ((*it->begin()) == '-')
	    {
	      std::cerr << " -";
	      atom = it->substr(1, it->length());
	    }
	  else
	    {
	      atom = *it;
	    }

	  std::cerr << atom << "==" << id2lid.left.at(id2name.right.at(atom)) << ", ";
	}
    }
  std::cerr << std::endl;
#endif // DEBUG

  for (boost::tokenizer<boost::char_separator<char> >::iterator it = answers.begin(); 
       it != answers.end(); ++it)
    {
      if (*it != " ")
	{
	  std::string atom;

	  if ((*it->begin()) == '-')
	    {
	      answer_stream << "-";
	      atom = it->substr(1, it->length());
	    }
	  else
	    {
	      atom = *it;
	    }

	  answer_stream << id2lid.left.at(id2name.right.at(atom)) << " 0" << std::endl;
	  count++;
	}
    }

  ClaspProcess cp;
  cp.spawn();
  std::ostream& os(cp.getOutput());
  
  os << "p cnf " << id2name.size() << " " << theory->size() + count << std::endl << global_sat_stream.str() << answer_stream.str();
  os.flush();
  cp.endoffile();

  std::string result;

  std::istream& is = cp.getInput();

  while (!is.eof())
    {
      getline(is, result);
      if (result == "s SATISFIABLE")
	{
	  ++pass;
	}
#ifdef DEBUG
      std::cerr << result << std::endl;
#endif
    }

  cp.close();
}

int main(int argc, char* argv[])
{
  total_answers = pass = 0;

  if (argc != 3)
    {
      std::cout << "Usage: " << argv[0] << " <theory_file_name> <answer_file_name>" << std::endl;
      return 1;
    }

  std::string theory_filename = argv[1];

  CnfBuilder<CnfGrammar> builder(theory, id2name);
  ParserDirector<CnfGrammar> parser_director(builder);
  parser_director.parse(theory_filename);
  create_local_id();


  DimacsVisitor dv(global_sat_stream);
  dv.visitTheory1(theory, id2lid);

  std::ifstream answer_file(argv[2]);
  std::string line;

#ifdef DEBUG
  print_name_ids();
#endif

  if (answer_file.is_open())
    {
      while (!answer_file.eof())
	{
	  getline(answer_file, line);
	  if (line != "")
	    {
	      ++total_answers;
	      check_answer(line);
	    }
	}
      answer_file.close();

      std::cerr << "Total answers  = " << total_answers << std::endl;
      std::cerr << "Answers passed = " << pass << std::endl;
    }
  else
    {
      std::cerr << "Unable to open file " << argv[2] << "!" << std::endl;
    }
}
