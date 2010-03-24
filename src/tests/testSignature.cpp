#include "Signature.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSignature"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE( testSignatureSimple )
{
  Signature sig1;

  // we are in ctx 1 and populate \Sigma_1 = "a1 b1 c1 d1"
  sig1.insert(Symbol("a1",1,1,1));
  sig1.insert(Symbol("b1",1,2,2));
  sig1.insert(Symbol("c1",1,3,3));
  sig1.insert(Symbol("d1",1,4,4));

  // max count
  std::size_t max = sig1.size();

  // then we parse kb_1 and look-up the local ids
  // say we parse "c1", we then get 3

  const SignatureBySym& sym = boost::get<Tag::Sym>(sig1);
  SignatureBySym::const_iterator sym_it = sym.find("c1");
  BOOST_CHECK_EQUAL( sym_it->sym, "c1" );
  BOOST_CHECK_EQUAL( sym_it->ctxId, std::size_t(1) );
  BOOST_CHECK_EQUAL( sym_it->localId, std::size_t(3) );
  BOOST_CHECK_EQUAL( sym_it->origId, std::size_t(3) );


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

  // then we parse br_1 and parse say (4:b4) which should should get
  sig1.insert(Symbol("b4",4,max + count++,2));
  // and parse say (2:d2) which should should get
  sig1.insert(Symbol("d2",2,max + count++,4));

  // we can then lookup all local ids which eventually show up from the sat solver

  const SignatureByLocal& local = boost::get<Tag::Local>(sig1);

  SignatureByLocal::const_iterator loc_it = local.find(5); // this is b4
  BOOST_CHECK_EQUAL( loc_it->sym, "b4" );
  BOOST_CHECK_EQUAL( loc_it->ctxId, std::size_t(4) );
  BOOST_CHECK_EQUAL( loc_it->localId, std::size_t(5) );
  BOOST_CHECK_EQUAL( loc_it->origId, std::size_t(2) );

  loc_it = local.find(3); // this is c1
  BOOST_CHECK_EQUAL( loc_it->sym, "c1" );
  BOOST_CHECK_EQUAL( loc_it->ctxId, std::size_t(1) );
  BOOST_CHECK_EQUAL( loc_it->localId, std::size_t(3) );
  BOOST_CHECK_EQUAL( loc_it->origId, std::size_t(3) );
}
