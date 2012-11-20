#undef BOOST_SPIRIT_DEBUG
#undef BOOST_SPIRIT_DEBUG_WS

#include "parser/QueryPlanParser.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp> 

#include <istream>
#include <fstream>

namespace
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

using namespace dmcs;

// state such that semantic actions can communicate
struct SemState
{
  //SemState(): qplan(), currentCtx(0) {}
  ContextQueryPlanMapPtr qplan;
  ContextID currentCtx;
};

//std::ostream& operator<<(std::ostream& o, const SemState& s)
//  { return o << "SemState("<< s.qplan << "," << s.currentCtx << ")"; }

struct StoreAndRememberContextId
{
  SemState& s;
  StoreAndRememberContextId(SemState& s): s(s) {}

  template<typename SourceAttributes, typename Ctx>
  void operator()(
    const SourceAttributes& source,
    Ctx& ctx,
    boost::spirit::qi::unused_type) const
  {
    // ctx_in_locals is a_
    ContextID& ctx_in_locals = boost::fusion::at_c<0>(ctx.locals);
    const int& parserattr = source;
    s.currentCtx = parserattr;
    ctx_in_locals = parserattr;
    //std::cerr << "encountering context " << s.currentCtx << std::endl;
  }
};

struct InsertIntoMap
{
  SemState& s;
  InsertIntoMap(SemState& s): s(s) {}

  template<typename SourceAttributes, typename Ctx>
  void operator()(
    const SourceAttributes& source,
    Ctx& ctx,
    boost::spirit::qi::unused_type) const
  {
    ContextQueryPlanMapPtr& qpm = s.qplan;
    if( !qpm )
      qpm.reset(new ContextQueryPlanMap);
    const std::vector<ContextQueryPlan>& qp = source;
    for(std::vector<ContextQueryPlan>::const_iterator it = qp.begin();
        it != qp.end(); ++it)
    {
      qpm->insert(std::make_pair(it->ctx, *it));
    }
  }
};

#if 0
struct AssignPtrOptional
{
  BeliefTablePtr& tgt;
  AssignPtrOptional(BeliefTablePtr& tgt): tgt(tgt) {}

  template<typename SourceAttributes, typename Ctx>
  void operator()(
    const boost::optional<BeliefTablePtr>& source,
    Ctx& ctx,
    boost::spirit::qi::unused_type) const
  {
    if( !!source )
    {
      tgt = source.get();
    }
  }
};
#endif

struct RegisterAndInsertIntoBeliefSet
{
  SemState& s;
  RegisterAndInsertIntoBeliefSet(SemState& s): s(s) {}

  template<typename SourceAttributes, typename Ctx>
  void operator()(
    const SourceAttributes& source,
    Ctx& ctx,
    boost::spirit::qi::unused_type) const
  {
    BeliefTablePtr& qpm = boost::fusion::at_c<0>(ctx.attributes);
    if( !qpm )
    {
      // create belief table if does not exist
      qpm = BeliefTablePtr(new BeliefTable);
    }

    const std::vector<fusion::vector2<IDAddress, std::vector<std::string> > >& inputs = source;
    std::vector<fusion::vector2<IDAddress, std::vector<std::string> > >::const_iterator itv;
    for(itv = inputs.begin(); itv != inputs.end(); ++itv)
    {
      const fusion::vector2<IDAddress, std::vector<std::string> >& input = *itv;
      const IDAddress address = fusion::at_c<0>(input);
      const std::vector<std::string>& tuples = fusion::at_c<1>(input);

      std::vector<std::string>::const_iterator it = tuples.begin();
      assert(it != tuples.end() && "atom must have at least one constant in tuple");
      std::string predicate;
      predicate += *it;
      it++;
      if( it != tuples.end() )
      {
        predicate += "(";
        for(;it != tuples.end(); ++it)
        {
          predicate += *it + ",";
        }
        predicate[predicate.size()-1] = ')';
      }
      //std::cerr << "got belief " << address << " with string '" << predicate << "'" << std::endl;

      Belief bel(s.currentCtx, address, predicate);
      qpm->storeWithID(bel, ID(bel.kind, bel.address));
    }
  }
};

struct lazy_get_impl
{
  template<typename X>
  struct result;

  template<typename T>
  struct result<boost::optional<T> >
  {
    typedef T type;
  };

  template<typename T>
  T operator()(const boost::optional<T>& optarg) const
  {
    //std::cerr << "doing lazy get impl for arg " << !!optarg << std::endl;
    //std::cerr << "doing lazy get impl for ARG " << optarg.get() << std::endl;
    return optarg.get();
  }
};

phoenix::function<lazy_get_impl> lazy_get;

template<typename Iterator, typename Skipper>
struct QueryPlanGrammar:
  qi::grammar<Iterator, Skipper>
{
  QueryPlanGrammar(SemState& state) : QueryPlanGrammar::base_type(start)
  {
    using qi::lit;
    using qi::eps;
    using qi::_1;
    using qi::int_;
    using qi::uint_;
    using qi::_val;
    using namespace qi::labels;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::insert;
    using phoenix::at_c;

//   Grammar of query plan
//   Context ID [
//     Constants:  [c1, ..., cn],
//     Categories: [[Category: cat1, Constants: [1, ...,n]], ..., [Category: catn, Constants: [1, ...,n]]],
//     Predicates: [[Pred: a1, Arity: 1], ..., [Pred: an, Arity: 2]]
//     Filters:    [[Name: f1, Pred: a1, Arguments:[[Position:1, Using:[cat1, ..., catn]], ..., []]], ...,[]]
//   ]

    start = (
             lit('[') >> contextqueryplan % ',' >> -(lit(',')) >> lit(']')
            ) [InsertIntoMap(state)];

    contextqueryplan = lit('{') >>
      lit("ContextId") >> ':' >> int_ /*[_a = _1]*/ [StoreAndRememberContextId(state)] >> 
      ',' >> lit("HostName") >> ':' >> hostname [_b = _1] >>
      ',' >> lit("Port") >> ':' >> int_ [_c = _1] >>
      -(',' >> lit("Constants") >> ':' >>
        //_b = construct<ConstantListPtr>(new_<ConstantList>(_1)) ?
        constants [_d = _1]) >>
      -(',' >> lit("ConstantCategories") >> ':' >>
        //_c = construct<CategoryListPtr>(new_<CategoryList>(_1)) ?
        categories [_e = _1]) >>
      -(',' >> lit("Predicates") >> ':' >>
        //_d = construct<PredicateListPtr>(new_<PredicateList>(_1)) ?
        predicates [_f = _1]) >>
      -(',' >> lit("Filters") >> ':' >>
        //_e = construct<FilterListPtr>(new_<FilterList>(_1)) ?
        filters [_g = _1]) >>
      (-(',' >> lit("LocalSignature") >> ':' >> signature ))
        [ if_( !!_1)
          [ _h = lazy_get(_1) ]
        ] >>
      (-(',' >> lit("InputSignature") >> ':' >> signature ))
        [ if_( !!_1)
          [ _i = lazy_get(_1) ]
        ] >>
      -(',' >> lit("OutputProjections") >> ':' >>
        outputprojections [_j = _1]) >> -(lit(',')) >>
    '}' >> eps [_val = construct<ContextQueryPlan>(
						   _a, _b, _c, _d, _e, _f, _g, _h, _i, _j)];

   signature = lit('{')
     >> (id_with_ground_tuple % ',') [RegisterAndInsertIntoBeliefSet(state)] 
     >> -(lit(',')) >> lit('}');

   id_with_ground_tuple %=
     uint_ >> lit(':') >> lit('[') >> ident % ',' >> lit(']');

   ident %= ( +(ascii::alnum) )
          | ( lit('"') >> qi::lexeme[*(ascii::char_ - '"')] >> lit('"') );

   ///TODO: grammar for URL. Now simplified by putting hostname between the quotes
   hostname %= lit('"') >> qi::lexeme[*(ascii::char_ - '"')] >> lit('"');

    /*
       constants %= lit("Constants:") >> '[' >> constant % ',' >> ']';
    constant %= +(alnum);

    categories %= lit("ConstantCategories:") >> '[' >> category % ',' >> ']';
    category = '[' >> lit("Category:") >> catSymbol [_a = _1] >> ',' >> constants [_b = construct<ConstantListPtr>(new_<ConstantList>(_1))] >> ']' >>
                eps [_val = construct<CategoryPtr>(new_<Category>(_a, _b))]; 
    catSymbol = +(alnum);
    
    predicates %= lit("Predicates:") >> '[' >> predicate % ',' >> ']';
    predicate = '[' >> lit("Pred:") >> predSymbol [_a = _1] >> ',' >> lit("Arity:") >> arity [_b = _1] >> ']' >>
		eps [_val = construct<PredicatePtr>(new_<Predicate>(_a, _b))];
    predSymbol %= +(alnum);
    arity %= int_;  

    filters %= lit("Filters:") >> '[' >> filter % ',' >> ']';
    filter = '[' >> lit("Name:") >> filterName [_a = _1] >>  ',' >> lit("Pred:") >> predSymbol [_b = _1] >> ',' >> 
                    lit("Arguments:") >> '[' >> argument [push_back(_c,_1)] >> (*(',' >> argument) [push_back(_c, _1)]) >> ']' >> ']' >>
	      eps [_val = construct<FilterPtr>(new_<Filter>(_a, _b, construct<FilterArgumentListPtr>(new_<FilterArgumentList>(_c))))];
    
    argument = '[' >> lit("Position:") >> arity [_a = _1] >> ',' >> 
    //                  lit("Using:") >> '[' >> catSymbol % ','  [_b = _1] >> ']'  >> ']' >>
	              lit("Using:") >> '[' >> catSymbol [push_back(_b, _1)] >> (*(',' >> catSymbol) [push_back(_b, _1)]) >> ']'  >> ']' >>
		eps [_val = construct<FilterArgumentPtr>(new_<FilterArgument>(_a, construct<CategorySymbolListPtr>(new_<CategorySymbolList>(_b))))];
    
    filterName %= +(alnum);

//    filterName = eps [_val = construct<FilterNamePtr>(new_<FilterName>())] >> (+(alnum) [push_back(*_val, _1)]);

*/
    #ifdef BOOST_SPIRIT_DEBUG
    BOOST_SPIRIT_DEBUG_NODE(start);
    BOOST_SPIRIT_DEBUG_NODE(contextqueryplan);
    BOOST_SPIRIT_DEBUG_NODE(constants);
    BOOST_SPIRIT_DEBUG_NODE(categories);
    BOOST_SPIRIT_DEBUG_NODE(predicates);
    BOOST_SPIRIT_DEBUG_NODE(filters);
    BOOST_SPIRIT_DEBUG_NODE(signature);
    BOOST_SPIRIT_DEBUG_NODE(id_with_ground_tuple);
    BOOST_SPIRIT_DEBUG_NODE(outputprojections);
    BOOST_SPIRIT_DEBUG_NODE(ident);
    #endif
  }
  
  qi::rule<Iterator, Skipper> start;

  qi::rule<Iterator, ContextQueryPlan(),
	   qi::locals<
	     ContextID, std::string, int, ConstantListPtr, ConstantCategoryListPtr,
	     PredicateArityMapPtr, FilterListPtr, BeliefTablePtr,
	     BeliefTablePtr, OutputProjectionMapPtr
	     >, Skipper> contextqueryplan;

  qi::rule<Iterator, ConstantListPtr(), Skipper> constants;
  //qi::rule<Iterator, Constant(), Skipper> constant;

  qi::rule<Iterator, ConstantCategoryListPtr(), Skipper> categories;
  //qi::rule<Iterator, CategoryPtr(), qi::locals<CategorySymbol, ConstantListPtr>, ascii::space_type> category;
  //qi::rule<Iterator, CategorySymbol(), Skipper> catSymbol;

  qi::rule<Iterator, PredicateArityMapPtr(), Skipper> predicates;  
  //qi::rule<Iterator, PredicatePtr(),qi::locals<PredSymbol, Arity>, ascii::space_type> predicate;
  //qi::rule<Iterator, PredSymbol(), Skipper> predSymbol;
  //qi::rule<Iterator, Arity(), Skipper> arity;

  qi::rule<Iterator, FilterListPtr(), Skipper> filters;
  //qi::rule<Iterator, FilterPtr(), qi::locals<FilterName, PredSymbol, FilterArgumentList>, Skipper> filter;
  //qi::rule<Iterator, FilterName(), Skipper> filterName;
  //qi::rule<Iterator, FilterArgumentPtr(), qi::locals<Arity, CategorySymbolList>, Skipper> argument;

  qi::rule<Iterator, BeliefTablePtr(), Skipper> signature;
  qi::rule<Iterator, fusion::vector2<IDAddress, std::vector<std::string> >(), Skipper> id_with_ground_tuple;

  qi::rule<Iterator, OutputProjectionMapPtr(), Skipper> outputprojections;

  qi::rule<Iterator, std::string(), Skipper> hostname;
  qi::rule<Iterator, std::string(), Skipper> ident;
};

template<typename Iterator>
struct SkipperGrammar:
  boost::spirit::qi::grammar<Iterator>
{
  SkipperGrammar():
    SkipperGrammar::base_type(ws)
  {
    using namespace boost::spirit;
    ws = ascii::space
       | qi::lexeme[ qi::char_('%') > *(qi::char_ - qi::eol) ];

    #ifdef BOOST_SPIRIT_DEBUG_WS
    BOOST_SPIRIT_DEBUG_NODE(ws);
    #endif
  }

  boost::spirit::qi::rule<Iterator> ws; 
};

}

namespace dmcs
{

ContextQueryPlanMapPtr
QueryPlanParser::parseFile(const std::string& infile)
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
QueryPlanParser::parseStream(std::istream& in)
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
QueryPlanParser::parseString(const std::string& str)
{
  std::string::const_iterator begIt = str.begin();
  std::string::const_iterator endIt = str.end();

  using ascii::space;
  using qi::phrase_parse;


  typedef SkipperGrammar<std::string::const_iterator> Skipper;

  Skipper skipper;
  SemState state;
  QueryPlanGrammar<std::string::const_iterator, Skipper> grammar(state);

  bool r = phrase_parse(begIt, endIt, grammar, skipper);
   
  if(r && begIt == endIt)
  {
    //std::cout << "Parsing succeeded\n";
    //std::cout << "Result is: " << state.qplan << std::endl;
    //std::cout << *state.qplan << std::endl;
    return state.qplan;
  }
  else
  {
    std::cout << "Parsing Failed\n";
    throw std::runtime_error("query plan parsing failed");
  }
}

} //namespace dmcs
