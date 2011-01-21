#include "network/connection.hpp"
#include "dmcs/Log.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testConnection"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testConnection )
{
  init_loggers("testConnection");

  boost::asio::io_service ios;
  connection c(ios);

}
