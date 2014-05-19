#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Wait
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>
#include <chrono>

using namespace masio;
using namespace std;
using namespace std::chrono;
namespace asio = boost::asio;

//------------------------------------------------------------------------------
system_clock::time_point now() { return system_clock::now(); }

//------------------------------------------------------------------------------
#define REQUIRE_DURATION(duration, reference) \
  BOOST_REQUIRE_LE( abs(duration_cast<milliseconds>(duration).count() \
                        - reference)\
                  , 1);

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_wait) {
  asio::io_service ios;

  unsigned int wait_duration = 500; // Half a second

  auto p = post(ios)
         > success<int>(10)
        >= [&ios, wait_duration](int a) {
          return masio::wait(ios, wait_duration)
               > success(2*a + 1.6);
        }
        >= [](float a) {
          return success<int>(a*2);
        };

  auto start = now();

  p.execute([start, wait_duration](result<int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>(), 43);
     REQUIRE_DURATION(now() - start, wait_duration);
     });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_cancel_wait) {
  asio::io_service ios;

  unsigned int wait_duration = 10*1000; // Ten seconds

  auto p = post(ios)
         > success<int>(10)
        >= [&ios, wait_duration](int a) {
          return masio::wait(ios, wait_duration)
               > success<float>(2*a + 1);
        }
        >= [](float a) {
            return success<int>(a+2);
        };

  auto start = now();

  p.execute([start](result<int> i) {
     BOOST_REQUIRE(i.is_error());
     BOOST_REQUIRE_EQUAL(i.error(), asio::error::operation_aborted);
     REQUIRE_DURATION(now() - start, 0);
     });

  int poll_count = 0;

  while(ios.run_one()) {
    p.cancel();
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);

}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_wait_and_may_fail) {
  asio::io_service ios;

  unsigned int wait_duration = 100; // Milliseconds

  auto p = post(ios)
         > success<int>(10)
        >= [&ios, wait_duration](int a) {
          return may_fail(masio::wait(ios, wait_duration)
                           > success<int>(2*a + 1));
        };

  auto start = now();

  p.execute([start, wait_duration](result<result<int>> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE(!i.value<0>().is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>().value<0>(), 21);
     REQUIRE_DURATION(now() - start, wait_duration);
     });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_cancel_wait_and_may_fail) {
  asio::io_service ios;

  unsigned int wait_duration = 10*1000; // Ten seconds

  auto p = post(ios)
         > success<int>(10)
        >= [&ios, wait_duration](int a) {
          return may_fail(masio::wait(ios, wait_duration)
                           > success<int>(2*a + 1));
        };

  auto start = now();

  p.execute([start](result<result<int>> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE(i.value<0>().is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>().error(), asio::error::operation_aborted);
     REQUIRE_DURATION(now() - start, 0);
     });

  int poll_count = 0;

  while(ios.run_one()) {
    p.cancel();
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

