#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Core
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <masio.h>
#include <iostream>

using namespace masio;
using namespace std;
namespace asio = boost::asio;

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(zero_binds) {
  auto p = success(10, 11);

  bool executed = false;

  p.execute([&executed](result<int, int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>(), 10);
     BOOST_REQUIRE_EQUAL(i.value<1>(), 11);
     executed = true;
     });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(one_bind) {
  auto p = success() >= []() { return success(1); };

  bool executed = false;

  p.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 1);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(one_bind_no_args) {
  auto p1 = success();
  auto p2 = p1 >= []() { return success(1); };

  bool executed = false;

  p2.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 1);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(one_bind_one_arg) {
  auto p1 = success(10);
  auto p2 = p1 >= [](int i) { return success(i+1); };

  bool executed = false;

  p2.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 11);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(one_bind_two_args) {
  auto p1 = success(10, 10);
  auto p2 = p1 >= [](int i, int j) { return success(i+j+1); };

  bool executed = false;

  p2.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 21);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(two_binds) {
  auto p1 = success<int>(10);
  auto p2 = p1 >= [](int i) { return success(i+1); };
  auto p3 = p2 >= [](int i) { return success(i+1); };

  bool executed = false;

  p3.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 12);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(two_binds_one_var) {
  auto p = success<int>(10) >= [](int i)
           { return success(i+1); } >= [](int i)
           { return success(i+1); };

  bool executed = false;

  p.execute([&executed](result<int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>(), 12);
     executed = true;
     });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
// Fail right at the beginning.
BOOST_AUTO_TEST_CASE(fail0) {
  auto p = fail<int>(asio::error::operation_aborted)
        >= [](int a) { return success<int>(a); };

  bool executed = false;

  p.execute([&executed](result<int> i) {
     BOOST_REQUIRE(i.is_error());
     BOOST_REQUIRE_EQUAL(i.error(), asio::error::operation_aborted);
     executed = true;
     });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(store_in_action_zero_binds) {
  action<int> p = success(10);

  bool executed = false;

  p.execute([&executed](result<int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>(), 10);
     executed = true;
     });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(store_in_action_one_bind) {
  action<int> p1 = success(10);
  action<int> p2 = p1 >= [](int i) { return success(i+1); };

  bool executed = false;

  p2.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 11);
      executed = true;
      });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(store_in_action_one_bind_one_var) {
  action<int> p = success(10) >= [](int i) { return success(i+1); };

  bool executed = false;

  p.execute([&executed](result<int> i) {
     BOOST_REQUIRE(!i.is_error());
     BOOST_REQUIRE_EQUAL(i.value<0>(), 11);
     executed = true;
     });

  BOOST_CHECK(executed);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(store_in_action_two_binds) {
  action<int> p1 = success(10);
  action<int> p2 = p1 >= [](int i) { return success(i+1); };
  action<int> p3 = p2 >= [](int i) { return success(i+1); };

  bool executed = false;

  p3.execute([&executed](result<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE_EQUAL(i.value<0>(), 12);
      executed = true;
      });

  BOOST_CHECK(executed);
}

