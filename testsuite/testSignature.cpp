#include "mcs/Signature.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSignature"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testSignatureSimple )
{
  Signature sig1;

  // we are in ctx 1 and populate \Sigma_1 = "a1 b1 c1 d1"
  sig1.insert(Symbol("a",1,1,1));
  sig1.insert(Symbol("b",1,2,2));
  sig1.insert(Symbol("c",1,3,3));
  sig1.insert(Symbol("d",1,4,4));

  // This guy should not be in
  sig1.insert(Symbol("c",1,5,5));

  // max count
  std::size_t max = sig1.size();

  // then we parse kb_1 and look-up the local ids
  // say we parse "d", we then get 4

  const SignatureBySym& sym = boost::get<Tag::Sym>(sig1);
  SignatureBySym::const_iterator sym_it = sym.find("d");
  BOOST_CHECK_EQUAL( sym_it->sym, "d" );
  BOOST_CHECK_EQUAL( sym_it->ctxId, std::size_t(1) );
  BOOST_CHECK_EQUAL( sym_it->localId, std::size_t(4) );
  BOOST_CHECK_EQUAL( sym_it->origId, std::size_t(4) );

  // query signature with a pair (symbol string, contextId)
  const SignatureBySymCtx& symctx = boost::get<Tag::SymCtx>(sig1);
  SignatureBySymCtx::const_iterator symctx_it = symctx.find(boost::make_tuple("c", 1));
  BOOST_CHECK_EQUAL( symctx_it->sym, "c" );
  BOOST_CHECK_EQUAL( symctx_it->ctxId, std::size_t(1) );
  BOOST_CHECK_EQUAL( symctx_it->localId, std::size_t(3) );
  BOOST_CHECK_EQUAL( symctx_it->origId, std::size_t(3) );

  // this is who we would populate our neighbor 2, if we access all of \Sigma_2 = "a2 b2 c2 d2 e2"
  //sig1.insert(Symbol("a2",2,5,1));
  //sig1.insert(Symbol("b2",2,6,2));
  //sig1.insert(Symbol("c2",2,7,3));
  //sig1.insert(Symbol("d2",2,8,4));
  //sig1.insert(Symbol("e2",2,9,5));

  // this is how we would populate our neighbor 4, if we access all of \Sigma_4 ="a4 b4 c4"
  //sig1.insert(Symbol("a4",4,10,1));
  //sig1.insert(Symbol("b4",4,11,2));
  //sig1.insert(Symbol("c4",4,12,3));

  // local count offset
  int count = 1;

  // then we parse br_1 and parse say (4:b) which should should get
  sig1.insert(Symbol("b",4,max + count++,2));
  // and parse say (2:d) which should should get
  sig1.insert(Symbol("d",2,max + count++,4));

  // we can then lookup all local ids which eventually show up from the sat solver

  const SignatureByLocal& local = boost::get<Tag::Local>(sig1);

  SignatureByLocal::const_iterator loc_it = local.find(5); // this is b at context 4
  BOOST_CHECK_EQUAL( loc_it->sym, "b" );
  BOOST_CHECK_EQUAL( loc_it->ctxId, std::size_t(4) );
  BOOST_CHECK_EQUAL( loc_it->localId, std::size_t(5) );
  BOOST_CHECK_EQUAL( loc_it->origId, std::size_t(2) );
}
