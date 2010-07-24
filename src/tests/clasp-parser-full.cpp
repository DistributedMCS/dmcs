// Incrementally parse result from clasp (full vesion)
// to compile: g++ -I/opt/local/include claspparser.cpp

#include <iostream>
#include <string>
#include <fstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/shared_ptr.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

typedef std::vector<int> IntVec;
typedef boost::shared_ptr<IntVec> IntVecPtr;
typedef std::string::const_iterator forward_iterator_type;

struct handle_int
{
  handle_int(IntVecPtr V_) 
    : V(V_)
  { }

  void
  operator()(int i, qi::unused_type, qi::unused_type) const
  {
    if (i > 0)
      {
	// turn on the i-th bit of V[0]. In real dmcs-clasp parsing, we
	// first need to look into the signature to find the original
	// context and id from the local id

	(*V)[0] |= (1 << i); 
      }
  }

  IntVecPtr V;
};

struct handle_model
{
  handle_model(bool& got_answer_)
    : got_answer(got_answer_)
  { }

  void
  operator()(qi::unused_type, qi::unused_type, qi::unused_type) const
  {
    got_answer = true;
  }
  
  bool& got_answer;
};


// The Grammar
template <typename Iterator>
struct ClaspResultGrammar : qi::grammar<Iterator, ascii::space_type >
{

  ClaspResultGrammar(IntVecPtr V_)
    : ClaspResultGrammar::base_type(start), V(V_), got_answer(false)
  {
    sentinel = qi::char_('0');
    
    literal = qi::int_ - sentinel;

    //model = +literal[phoenix::push_back(phoenix::ref(*V), _1)] >> sentinel;
    model = qi::char_('v') >> +literal[handle_int(V)] >> sentinel;

    solution = qi::char_('s') >> (qi::string("SATISFIABLE") |
				  qi::string("UNSATISFIABLE") | 
				  qi::string("UNKNOWN"));

    comment = qi::char_('c') >> *(qi::char_ - qi::eol);

    start = comment | solution | model[handle_model(got_answer)];
  }

  qi::rule<Iterator, ascii::space_type > sentinel;
  qi::rule<Iterator, int(), ascii::space_type > literal;
  qi::rule<Iterator, ascii::space_type > model;
  qi::rule<Iterator, ascii::space_type > solution;
  qi::rule<Iterator, ascii::space_type > comment;
  qi::rule<Iterator, ascii::space_type > start;

  IntVecPtr V;
  bool got_answer;
};


int 
main()
{
  std::ifstream claspfile("result.sat");

  if (claspfile.is_open())
    {
 
      std::string line;

      while (!claspfile.eof())
	{
	  bool complete = false;
	  IntVecPtr v(new IntVec(3, 0));
	  ClaspResultGrammar<forward_iterator_type> crg(v);
	  
	  while (!complete && !claspfile.eof())
	    {
	      getline(claspfile, line);
	      std::cout << "Processing: " << line << std::endl;

	      
	      forward_iterator_type beg = line.begin();
	      forward_iterator_type end = line.end();
	      
	      complete = qi::phrase_parse(beg, end, crg, ascii::space);
	      if (complete)
		{
		  std::cout << "Parse succeeded!" << std::endl;
		  if (crg.got_answer)
		    {
		      std::cout << "Got answer!" << std::endl;
		    }
		  std::copy(v->begin(), v->end(), std::ostream_iterator<int>(std::cout, " "));
		  std::cout << std::endl;
		}
	      else
		{
		  std::cout << "Parse failed!" << std::endl;
		}
	    }
	}
    }
  else
    {
      std::cout << "Unable to open result.sat" << std::endl;
    }
}
