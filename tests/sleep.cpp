#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Sleep
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>
#include <chrono>

using namespace masio;
using namespace std;
using namespace std::chrono;
namespace asio = boost::asio;
typedef shared_ptr<State> StatePtr;

//------------------------------------------------------------------------------
system_clock::time_point now() { return system_clock::now(); }

//------------------------------------------------------------------------------
#define REQUIRE_DURATION(duration, reference) \
  BOOST_REQUIRE_LE( abs(duration_cast<milliseconds>(duration).count() \
                        - reference)\
                  , 1);
//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_sleep) {
  asio::io_service ios;

  unsigned int wait_time = 500; // Half a second

  StatePtr state = make_shared<State>();

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  })
  ->bind<float>([&ios, wait_time](int a) {
    return sleep<float>(ios, wait_time, [a]() {
      return success<float>(2*a + 1);
      });
  })
  ->bind<int>([](float a) {
      return success<int>(a+2);
  });

  auto start = now();

  p->run(state, [start, wait_time](Error<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value(), 23);
      REQUIRE_DURATION(now() - start, wait_time);
      });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);

}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_cancel_sleep) {
  asio::io_service ios;

  StatePtr state = make_shared<State>();

  unsigned int wait_time = 10*10000; // Ten seconds

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  })
  ->bind<float>([&ios, wait_time](int a) {
    return sleep<float>(ios, wait_time, [a]() {
      return success<float>(2*a + 1);
      });
  })
  ->bind<int>([](float a) {
      return success<int>(a+2);
  });

  auto start = now();

  p->run(state, [start](Error<int> i) {
      BOOST_REQUIRE(i.is_error());
      BOOST_REQUIRE(i.error() == asio::error::operation_aborted);
      REQUIRE_DURATION(now() - start, 0);
      });

  int poll_count = 0;

  while(ios.run_one()) {
    state->cancel();
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);

}

