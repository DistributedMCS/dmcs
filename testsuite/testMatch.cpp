#include "dyndmcs/Match.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testMatch"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testMatch )
{
  MatchTable m1;
  m1.insert(Match(1, 2, 3, 4, 0.8));
  m1.insert(Match(1, 2, 3, 5, 0.7));
  m1.insert(Match(1, 2, 3, 3, 0.6));
  m1.insert(Match(2, 2, 3, 5, 0));

  // this guy must not be inserted
  m1.insert(Match(2, 2, 3, 5, 0.5));

  std::cerr << "Total number of matches: " << m1.size() << std::endl;
  BOOST_CHECK_EQUAL(m1.size(), 4);

  const MatchTableBySrcSym& ss = boost::get<Tag::SrcSym>(m1);
  MatchTableBySrcSym::const_iterator low = ss.lower_bound(boost::make_tuple(1, 2));
  MatchTableBySrcSym::const_iterator up  = ss.upper_bound(boost::make_tuple(1, 2));

  std::cerr << "Number of matches found: " << std::distance(low, up) << std::endl;
  BOOST_CHECK_EQUAL(std::distance(low, up), 3);

  std::cerr << "Matches found: " << std::endl;
  for (; low != up; ++low)
    {
      std::cerr << *low << std::endl;
    }

  low = ss.lower_bound(boost::make_tuple(1, 3));
  up  = ss.upper_bound(boost::make_tuple(1, 3));

  std::cerr << "Number of matches found: " << std::distance(low, up) << std::endl;  
  BOOST_CHECK_EQUAL(std::distance(low, up), 0);

  const MatchTableBySrcSymTarImg& ssti = boost::get<Tag::SrcSymTarImg>(m1);
  MatchTableBySrcSymTarImg::const_iterator it = ssti.find(boost::make_tuple(2, 2, 3, 5));
  std::cerr << "Query the quality: " << *it << std::endl;
  BOOST_CHECK_EQUAL(it->quality, 0);

  std::istringstream iss("(3, 4, 5, 6, 1)");

  iss >> m1;

  MatchTable::const_iterator last = --m1.end();

  std::cerr << "Reading operator gave: " << *last << std::endl;
  BOOST_CHECK_EQUAL(last->srcCtx,  3);
  BOOST_CHECK_EQUAL(last->sym,     4);
  BOOST_CHECK_EQUAL(last->tarCtx,  5);
  BOOST_CHECK_EQUAL(last->img,     6);
  BOOST_CHECK_EQUAL(last->quality, 1);
}

// Local Variables:
// mode: C++
// End:

