#include "dyndmcs/Variable.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testVariable"
#include <boost/test/unit_test.hpp>

using namespace dmcs;

template<typename T>
std::string
binary(T num)
{
  std::string tmp = "";
  for (std::size_t i = 0; i < sizeof(T) * 8; ++i)
    {
      if (num & (1 << i))
	{
	  tmp = "1" + tmp;
	}
      else
	{
	  tmp = "0" + tmp;
	}
    }
  
  return tmp;
}

BOOST_AUTO_TEST_CASE ( testVariable )
{
  std::size_t context_id = 3;
  std::size_t variable_id = 15;
  ContextTerm composition = 0;

  composition = ctxVar(composition);
  std::cerr <<"composition: " << composition << "\t"<<  binary<ContextTerm>(composition) << std::endl;

  std::cerr << std::endl;

  composition = ctxVarTerm(context_id, variable_id);
  
  std::cerr <<"composition: " << composition << "\t"<<  binary<ContextTerm>(composition) << std::endl;
  std::cerr <<"is variable: " << isCtxVar(composition) << std::endl;
  std::cerr <<"context id : " << ctxID(composition) << std::endl;
  std::cerr <<"variable id: " << varID(composition) << std::endl;

  BOOST_CHECK_EQUAL(isCtxVar(composition), IS_VARIABLE);
  BOOST_CHECK_EQUAL(ctxID(composition), context_id);
  BOOST_CHECK_EQUAL(varID(composition), variable_id);

  SchematicBelief sb = 0;
  sb = sBeliefType(sb, IS_EXACT_SCHEMATIC_BELIEF);
  sb = sBelief(sb, 95);

  std::cerr <<"sbridge belief: " << sb << "\t"<<  binary<SchematicBelief>(sb) << std::endl;
  std::cerr <<"sbelief type:   " << sBeliefType(sb) << std::endl;
  std::cerr <<"belief content: " << sBelief(sb) << std::endl;

  BOOST_CHECK_EQUAL(sBeliefType(sb), IS_EXACT_SCHEMATIC_BELIEF);
  BOOST_CHECK_EQUAL(sBelief(sb), 95);
  
}
