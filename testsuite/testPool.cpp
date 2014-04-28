#include "dmcs/Log.h"
#include "mcs/BaseType.h"
#include "mcs/Pool.h"
#include "network/ConcurrentMessageQueue.h"


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testPool"
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>

using namespace dmcs;



struct MyType : public BaseType
{
  MyType(const std::string& n, std::size_t a)
    : BaseType(), name(n), age(a)
  { }

  std::string name;
  std::size_t age;
};



void
run_release_mem(Pool* pool, ConcurrentMessageQueue* cmq)
{
  std::size_t count = 0;
  std::stringstream out;

  while (1)
    {
      // wait at cmq for the next chunk to release
      MyType* mt_ptr = 0;
      std::size_t recvd = 0;
      unsigned int p = 0;
      void* ptr = static_cast<void*>(&mt_ptr);
      
      cmq->receive(ptr, sizeof(mt_ptr), recvd, p);

      assert (recvd == sizeof(mt_ptr));

      // sleep for some time before releasing the memory,
      // just to block the malloc a bit
      boost::posix_time::milliseconds n(500);
      boost::this_thread::sleep(n);

      out << "tweety" << count;

      BOOST_CHECK_EQUAL(mt_ptr->name, out.str());
      BOOST_CHECK_EQUAL(mt_ptr->age, count+5);

      pool->free(mt_ptr);
      count++;
      out.str("");
    }
}



BOOST_AUTO_TEST_CASE ( testPool )
{
  Pool* pool = new Pool(2, sizeof(MyType));
  ConcurrentMessageQueue* cmq = new ConcurrentMessageQueue(5);

  boost::thread t(boost::bind(run_release_mem, pool, cmq));

  std::stringstream out;

  for (std::size_t i = 0; i < 10; ++i)
    {
      // ask for some memory from the pool,
      // might have to wait once the pool is all used
      MyType* mt;
      out << "tweety" << i;
      mt = new (pool) MyType(out.str(), i+5);
      out.str("");

      // send to cmq so that the other thread can release this memory chunk
      cmq->send(&mt, sizeof(mt), 0);
    }
  boost::posix_time::milliseconds n(3000);
  boost::this_thread::sleep(n);
  t.interrupt();
  t.join();
}
