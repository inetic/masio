#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Post
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <masio.h>
#include <iostream>

using namespace masio;
using namespace std;
namespace asio = boost::asio;
typedef shared_ptr<Canceler> CancelerPtr;

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(zero_binds_one_post) {
  asio::io_service ios;

  CancelerPtr canceler = make_shared<Canceler>();

  auto p = post(ios, []() {
    return success(10);
  });

  p.run(canceler, [](Error<int> i) {
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

  CancelerPtr canceler = make_shared<Canceler>();

  auto p = post(ios, []() {
    return success(10);
  })
  >= [&ios](int a) {
    return post(ios, [a]() { return success(2*a + 1); });
  }
  >= [](float a) {
      return success<int>(a+2);
  };

  p.run(canceler, [](Error<int> i) {
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

  CancelerPtr canceler = make_shared<Canceler>();

  auto p = post(ios, []() {
    return success<int>(10);
  })
  >= [&ios](int a) {
    return fail<float>(operation_aborted);
  }
  >= [](float a) {
      return success<int>(a+2);
  };

  p.run(canceler, [](Error<int> i) {
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

  CancelerPtr canceler = make_shared<Canceler>();

  auto p = post(ios, []() {
    return success<int>(10);
  })
  >= [&ios](int a) {
    return post(ios, [a]() {
      return success<float>(2*a + 1);
    });
  }
  >= [](float a) {
    return fail<int>(asio::error::operation_aborted);
  };

  p.run(canceler, [](Error<int> i) {
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

  CancelerPtr canceler = make_shared<Canceler>();

  bool first_executed  = false;
  bool second_executed = false;

  auto p = post(ios, [&first_executed]() {
    first_executed = true;
    return success<int>(10);
  })
  >= [&second_executed, &ios](int a) {
    return post(ios, [a, &second_executed]() {
      second_executed = true;
      return success<float>(2*a + 1);
      });
  };

  bool executed = false;

  p.run(canceler, [&executed](Error<float> i) {
     executed = true;
     BOOST_REQUIRE(i.is_error());
     });

  int poll_count = 0;

  while(ios.poll_one()) {
    canceler->cancel();
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
  BOOST_REQUIRE(first_executed);
  BOOST_REQUIRE(!second_executed);
}

//------------------------------------------------------------------------------
