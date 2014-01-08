#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Sleep
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>
#include <chrono>

//------------------------------------------------------------------------------
using namespace masio;
using namespace std;
namespace asio = boost::asio;
using namespace std::chrono;
typedef shared_ptr<Canceler> CancelerPtr;

//------------------------------------------------------------------------------
system_clock::time_point now() { return system_clock::now(); }

std::ostream& operator<<(std::ostream& os, const system_clock::time_point& t) {
  return os << t.time_since_epoch() / seconds(1);
}

//------------------------------------------------------------------------------
#define REQUIRE_DURATION(duration, reference) \
  BOOST_REQUIRE_LE( abs(duration_cast<milliseconds>(duration).count() \
                        - reference)\
                  , 1);
//--------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_all) {
  asio::io_service ios;

  CancelerPtr canceler = make_shared<Canceler>();

  Cont<int>::Ptr p1 = post<int>(ios, []() { return success<int>(11); });
  Cont<int>::Ptr p2 = post<int>(ios, []() { return success<int>(22); });

  All<int>::Ptr p = all<int>(p1, p2);

  bool executed = false;

  p->run(canceler, [&executed](Error<std::vector<Error<int>>> ers) {
      BOOST_REQUIRE(!ers.is_error());

      const vector<Error<int>>& rs = ers.value();

      BOOST_REQUIRE_EQUAL(rs.size(), 2);

      BOOST_REQUIRE(rs[0].is_value());
      BOOST_REQUIRE(rs[1].is_value());

      BOOST_REQUIRE_EQUAL(rs[0].value(), 11);
      BOOST_REQUIRE_EQUAL(rs[1].value(), 22);

      executed = true;
      });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

//--------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_all_sleep) {
  asio::io_service ios;

  CancelerPtr canceler = make_shared<Canceler>();

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  Cont<Time>::Ptr p0 = sleep<Time>(ios, duration0, []() {
      return success<Time>(now());
      });

  Cont<Time>::Ptr p1 = sleep<Time>(ios, duration1, []() {
      return success<Time>(now());
      });

  All<Time>::Ptr p = all<Time>(p0, p1);

  bool executed = false;

  auto start = now();

  p->run(canceler, [&executed, start, duration0, duration1]
                   (Error<std::vector<Error<Time>>> ers) {
      BOOST_REQUIRE(!ers.is_error());

      const vector<Error<Time>>& rs = ers.value();

      BOOST_REQUIRE_EQUAL(rs.size(), 2);

      BOOST_REQUIRE(rs[0].is_value());
      BOOST_REQUIRE(rs[1].is_value());

      auto end = now();
      auto t0  = rs[0].value();
      auto t1  = rs[1].value();

      REQUIRE_DURATION(t0 - start, duration0);
      REQUIRE_DURATION(t1 - start, duration1);
      REQUIRE_DURATION(end - start, max(duration0, duration1));

      executed = true;
      });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

//--------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_all_sleep_and_cancel) {
  asio::io_service ios;

  CancelerPtr canceler = make_shared<Canceler>();

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  Cont<Time>::Ptr p0 = sleep<Time>(ios, duration0, [canceler]() {
      canceler->cancel();
      return success<Time>(now());
      });

  Cont<Time>::Ptr p1 = sleep<Time>(ios, duration1, []() {
      return success<Time>(now());
      });

  All<Time>::Ptr p = all<Time>(p0, p1);

  bool executed = false;

  auto start = now();

  p->run(canceler, [&executed, start, duration0, duration1]
                   (Error<std::vector<Error<Time>>> ers) {
      BOOST_REQUIRE(!ers.is_error());

      const vector<Error<Time>>& rs = ers.value();

      BOOST_REQUIRE_EQUAL(rs.size(), 2);

      BOOST_REQUIRE(rs[0].is_value());
      BOOST_REQUIRE(rs[1].is_error());

      auto end = now();
      auto t0  = rs[0].value();
      auto e   = rs[1].error();

      REQUIRE_DURATION(t0 - start, duration0);
      REQUIRE_DURATION(end - start, min(duration0, duration1));
      BOOST_REQUIRE_EQUAL(e, asio::error::operation_aborted);

      executed = true;
      });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

//--------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_all_sleep_and_cancel_subcancelers) {
  asio::io_service ios;

  CancelerPtr canceler    = make_shared<Canceler>();
  CancelerPtr p1_canceler = make_shared<Canceler>();

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  Cont<Time>::Ptr p0 = sleep<Time>(ios, duration0, [p1_canceler]() {
      p1_canceler->cancel();
      return success<Time>(now());
      })
      ->bind<Time>([](Time t) { return success(t); });

  Cont<Time>::Ptr p1 = with_canceler<Time>( p1_canceler
                                          , sleep<Time>(ios, duration1, []() {
      return success<Time>(now());
      }))
      ->bind<Time>([](Time t) { return success(t); });

  All<Time>::Ptr p = all<Time>(p0, p1);

  bool executed = false;

  auto start = now();

  p->run(canceler, [&executed, start, duration0, duration1]
                  (Error<std::vector<Error<Time>>> ers) {
      BOOST_REQUIRE(!ers.is_error());

      const vector<Error<Time>>& rs = ers.value();

      BOOST_REQUIRE_EQUAL(rs.size(), 2);

      BOOST_REQUIRE(rs[0].is_value());
      BOOST_REQUIRE(rs[1].is_error());

      auto end = now();
      auto t0  = rs[0].value();
      auto e   = rs[1].error();

      REQUIRE_DURATION(t0 - start, duration0);
      REQUIRE_DURATION(end - start, min(duration0, duration1));
      BOOST_REQUIRE_EQUAL(e, asio::error::operation_aborted);

      executed = true;
      });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

