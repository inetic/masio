#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Post
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <masio.h>
#include <iostream>

using namespace masio;
using namespace std;
namespace asio = boost::asio;

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(zero_binds_one_post) {
  asio::io_service ios;

  Canceler canceler;

  auto p = post(ios) > success(10);

  p.execute(canceler, [](Error<int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(*i, 10);
     });

  int poll_count = 0;

  while(ios.poll_one()) { ++poll_count; }

  BOOST_REQUIRE(poll_count == 1);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(binds_and_posts) {
  asio::io_service ios;

  Canceler canceler;

  auto p = post(ios)
         > success(10)
        >= [&ios](int a) {
          return post(ios)  > success(2*a + 1);
        }
        >= [](float a) {
            return success<int>(a+2);
        };

  p.execute(canceler, [](Error<int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(*i, 23);
     });

  int poll_count = 0;

  while(ios.poll_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(poll_count == 2);
}

//------------------------------------------------------------------------------
// Fail at the middle of the computation.
BOOST_AUTO_TEST_CASE(fail1) {
  using asio::error::operation_aborted;

  asio::io_service ios;

  Canceler canceler;

  auto p = post(ios)
         > success<int>(10)
        >= [&ios](int a) {
          return fail<float>(operation_aborted);
        }
        >= [](float a) {
            return success<int>(a+2);
        };

  p.execute(canceler, [](Error<int> i) {
     BOOST_REQUIRE(i.is_error());
     BOOST_REQUIRE_EQUAL(i.error(), operation_aborted);
     });

  int poll_count = 0;

  while(ios.poll_one()) { ++poll_count; }

  BOOST_REQUIRE(poll_count == 1);
}

//------------------------------------------------------------------------------
// Fail at the end of the computation.
BOOST_AUTO_TEST_CASE(fail2) {
  asio::io_service ios;

  Canceler canceler;

  auto p = post(ios)
         > success<int>(10)
        >= [&ios](int a) {
          return post(ios) > success<float>(2*a + 1);
        }
        >= [](float a) {
          return fail<int>(asio::error::operation_aborted);
        };

  p.execute(canceler, [](Error<int> i) {
     BOOST_REQUIRE(i.is_error());
     BOOST_REQUIRE_EQUAL(i.error(), asio::error::operation_aborted);
     });

  int poll_count = 0;

  while(ios.poll_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(poll_count == 2);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(canceling) {
  asio::io_service ios;

  Canceler canceler;

  bool first_executed  = false;
  bool second_executed = false;

  auto p = post(ios)
        >= [&first_executed](none_t) {
             first_executed = true;
             return success<int>(10);
           }
        >= [&second_executed, &ios](int a) {
           return post(ios)
               >= [a, &second_executed](none_t) {
                    second_executed = true;
                    return success<float>(2*a + 1);
                  };
        };

  bool executed = false;

  p.execute(canceler, [&executed](Error<float> i) {
     executed = true;
     BOOST_REQUIRE(i.is_error());
     });

  int poll_count = 0;

  while(ios.poll_one()) {
    canceler.cancel();
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
  BOOST_REQUIRE(first_executed);
  BOOST_REQUIRE(!second_executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(forever_test) {
  asio::io_service ios;

  Canceler canceler;

  unsigned int call_count = 0;

  auto p = post(ios)
        >= [&call_count](none_t) {
             call_count++;
             return success(none);
           };

  bool executed = false;

  forever(p).execute(canceler, [&executed](Error<none_t> i) {
     executed = true;
     BOOST_REQUIRE(i.is_error());
     });

  int poll_count = 0;

  while(ios.poll_one()) {
    ++poll_count;

    if (poll_count == 5) {
      canceler.cancel();
    }
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 6);
  BOOST_REQUIRE_EQUAL(call_count, 5);
}

//------------------------------------------------------------------------------
