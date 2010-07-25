// Incrementally parse result from clasp (full vesion)

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testClaspOnlineParser"
#include <boost/test/unit_test.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

typedef std::vector<int> IntVec;
typedef boost::shared_ptr<IntVec> IntVecPtr;


struct handle_int
{
  IntVecPtr V;

  handle_int(const IntVecPtr& V_) 
    : V(V_)
  { 
    assert(V->size() > 0);
  }


  void
  operator() (int i, qi::unused_type, qi::unused_type) const
  {
    if (i > 0)
      {
	// turn on the i-th bit of V[0]. In real dmcs-clasp parsing, we
	// first need to look into the signature to find the original
	// context and id from the local id

	int& b = V->at(0);

	b |= (1 << i); 
      }
  }

};



struct handle_model
{
  bool& got_answer;

  handle_model(bool& got_answer_)
    : got_answer(got_answer_)
  { }

  void
  operator() (qi::unused_type, qi::unused_type, qi::unused_type) const
  {
    got_answer = true;
  }
};


// The Grammar
template <typename Iterator>
struct ClaspResultGrammar : qi::grammar<Iterator, ascii::space_type >
{
  qi::rule<Iterator, ascii::space_type > sentinel;
  qi::rule<Iterator, int(), ascii::space_type > literal;
  qi::rule<Iterator, ascii::space_type > partial_model;
  qi::rule<Iterator, ascii::space_type > model;
  qi::rule<Iterator, ascii::space_type > solution;
  qi::rule<Iterator, ascii::space_type > comment;
  qi::rule<Iterator, ascii::space_type > start;

  IntVecPtr V;
  bool got_answer;


  ClaspResultGrammar(IntVecPtr V_)
    : ClaspResultGrammar::base_type(start), V(V_), got_answer(false)
  {
    sentinel = qi::char_('0');
    
    literal = qi::int_ - sentinel;

    partial_model = qi::char_('v') >> +literal[handle_int(V)];

    model = qi::char_('v') >> +literal[handle_int(V)] >> sentinel;

    solution = qi::char_('s') >> (qi::string("SATISFIABLE") |
				  qi::string("UNSATISFIABLE") | 
				  qi::string("UNKNOWN"));

    comment = qi::char_('c') >> *(qi::char_ - qi::eol);

    // order really matters here
    start = model[handle_model(got_answer)] | partial_model | comment | solution;
  }
};




BOOST_AUTO_TEST_CASE( testClaspIncrementalParser )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string resultfile(ex);
  resultfile += "/result.sat";

  std::ifstream claspfile(resultfile.c_str());

  if (claspfile.is_open())
    {
      std::string line;

      while (!claspfile.eof())
	{
	  IntVecPtr v(new IntVec(3, 0));

	  typedef std::string::const_iterator forward_iterator_type;
	  ClaspResultGrammar<forward_iterator_type> crg(v);
	  
	  while (!crg.got_answer && !claspfile.eof())
	    {
	      std::getline(claspfile, line);
	      std::cout << "Processing: " << line << std::endl;

	      forward_iterator_type beg = line.begin();
	      forward_iterator_type end = line.end();
	      
	      bool succeeded = qi::phrase_parse(beg, end, crg, ascii::space);
	      if (succeeded)
		{
		  std::cout << "Parsing succeeded!" << std::endl;
		  if (crg.got_answer)
		    {
		      std::cout << "Got FULL answer:" << std::endl;

		    }
		  else
		    {
		      std::cout << "Got PARTIAL answer or (solution) or (comment)" << std::endl;
		    }

		  std::copy(v->begin(), v->end(), std::ostream_iterator<int>(std::cout, " "));
		  std::cout << std::endl;
		}
	      else
		{
		  std::cout << "Parsing failed!" << std::endl;
		}
	    }
	}
    }
  else
    {
      std::cout << "Unable to open result.sat" << std::endl;
    }
}
