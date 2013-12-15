#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SimpleRun
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <masio.h>
#include <iostream>

using namespace masio;
using namespace std;
namespace asio = boost::asio;

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(zeroBinds) {
  Cont<int>::Ptr p = success<int>(10);

  bool executed = false;

  p->run([&executed](Error<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE(i.value() == 10);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(zeroBindsOnePost) {
  asio::io_service ios;

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  });

  p->run([](Error<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE(i.value() == 10);
      });

  int poll_count = 0;

  while(ios.poll_one()) { ++poll_count; }

  BOOST_REQUIRE(poll_count == 1);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(bindsAndPosts) {
  asio::io_service ios;

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  })
  ->bind<float>([&ios](int a) {
    return post<float>(ios, [a]() {
      return success<float>(2*a + 1);
      });
  })
  ->bind<int>([](float a) {
      return success<int>(a+2);
  });

  p->run([](Error<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE(i.value() == 23);
      });

  int poll_count = 0;

  while(ios.poll_one()) { ++poll_count; }

  BOOST_REQUIRE(poll_count == 2);
}

//------------------------------------------------------------------------------
// End right at the beginning.
BOOST_AUTO_TEST_CASE(fail0) {
  Cont<int>::Ptr p = fail<int>(asio::error::operation_aborted)
    ->bind<int>([](int a) { return success<int>(a); });

  bool executed = false;

  p->run([&executed](Error<int> i) {
      BOOST_REQUIRE(i.is_error());
      BOOST_REQUIRE(i.error() == asio::error::operation_aborted);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
// End at the middle of the computation.
BOOST_AUTO_TEST_CASE(fail1) {
  using asio::error::operation_aborted;

  asio::io_service ios;

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  })
  ->bind<float>([&ios](int a) {
    return fail<float>(operation_aborted);
  })
  ->bind<int>([](float a) {
      return success<int>(a+2);
  });

  p->run([](Error<int> i) {
      BOOST_REQUIRE(i.is_error());
      BOOST_REQUIRE(i.error() == operation_aborted);
      });

  int poll_count = 0;

  while(ios.poll_one()) { ++poll_count; }

  BOOST_REQUIRE(poll_count == 1);
}

//------------------------------------------------------------------------------
// End at the end of the computation.
BOOST_AUTO_TEST_CASE(fail2) {
  asio::io_service ios;

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  })
  ->bind<float>([&ios](int a) {
    return post<float>(ios, [a]() {
      return success<float>(2*a + 1);
      });
  })
  ->bind<int>([](float a) {
      return fail<int>(asio::error::operation_aborted);
  });

  p->run([](Error<int> i) {
      BOOST_REQUIRE(i.is_error());
      BOOST_REQUIRE(i.error() == asio::error::operation_aborted);
      });

  int poll_count = 0;

  while(ios.poll_one()) { ++poll_count; }

  BOOST_REQUIRE(poll_count == 2);
}

//------------------------------------------------------------------------------
