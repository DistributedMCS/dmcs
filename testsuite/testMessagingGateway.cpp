#include "network/MessageQueueFactory.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testMessagingGateway"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;


BOOST_AUTO_TEST_CASE( testMessagingGateway )
{
  boost::shared_ptr<MessagingGateway<BeliefState,Conflict> > mg =
    MessageQueueFactory().createMessagingGateway(0, 5);
}
