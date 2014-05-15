#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE All
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>
#include <chrono>

//------------------------------------------------------------------------------
using namespace masio;
using namespace std;
using namespace boost::asio::error;
namespace asio = boost::asio;
using namespace std::chrono;

//------------------------------------------------------------------------------
system_clock::time_point now() { return system_clock::now(); }

std::ostream& operator<<(std::ostream& os, const system_clock::time_point& t) {
  return os << t.time_since_epoch() / seconds(1);
}

//------------------------------------------------------------------------------
#define REQUIRE_DURATION(duration, reference) \
  BOOST_REQUIRE_LE( abs(duration_cast<milliseconds>(duration).count() \
                        - (reference))\
                  , 1);
//--------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_all) {
  asio::io_service ios;

  auto p1 = post(ios) >= []() { return success(11); };
  auto p2 = post(ios) >= []() { return success(22); };

  auto p = all(p1, p2);

  bool executed = false;

  using Result = result<result<int>, result<int>>;

  p.execute([&executed](Result rs) {
     BOOST_REQUIRE(!rs.is_error());

     const auto& v0 = rs.value<0>();
     const auto& v1 = rs.value<1>();

     BOOST_REQUIRE(v0.is_value());
     BOOST_REQUIRE(v1.is_value());

     BOOST_REQUIRE_EQUAL(v0.value<0>(), 11);
     BOOST_REQUIRE_EQUAL(v1.value<0>(), 22);

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
BOOST_AUTO_TEST_CASE(test_all_wait) {
  using masio::wait;

  asio::io_service ios;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0) >= []() { return success(now()); };
  auto p1 = wait(ios, duration1) >= []() { return success(now()); };

  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = result<result<Time>, result<Time>>;

  p.execute([&executed, start, duration0, duration1](Results rs) {
     BOOST_REQUIRE(!rs.is_error());

     const auto& r0 = rs.value<0>();
     const auto& r1 = rs.value<1>();

     BOOST_REQUIRE(r0.is_value());
     BOOST_REQUIRE(r1.is_value());

     auto end = now();
     const auto& t0 = r0.value<0>();
     const auto& t1 = r1.value<0>();

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
BOOST_AUTO_TEST_CASE(test_all_wait_and_cancel) {
  using masio::wait;

  asio::io_service ios;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p1 = wait(ios, duration1) >= []() { return success(now()); };

  auto p0 = wait(ios, duration0)
         >= [&p1]() {
              p1.cancel();
              return success(now());
            };


  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = result<result<Time>, result<Time>>;

  p.execute([&executed, start, duration0, duration1](Results ers) {
     BOOST_REQUIRE(!ers.is_error());

     const auto& r0 = ers.value<0>();
     const auto& r1 = ers.value<1>();

     BOOST_REQUIRE(r0.is_value());
     BOOST_REQUIRE(r1.is_error());

     auto end = now();
     auto t0  = r0.value<0>();
     auto e   = r1.error();

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
BOOST_AUTO_TEST_CASE(test_all_or_none_wait_and_fail) {
  using masio::wait;

  asio::io_service ios;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0) >= []() { return fail<Time>(fault); };
  auto p1 = wait(ios, duration1) >= []() { return success(now()); };

  auto p = all_or_none(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = result<result<Time>, result<Time>>;

  p.execute([&executed, start, duration0, duration1](Results ers) {
     BOOST_REQUIRE(!ers.is_error());

     const auto& r0 = ers.value<0>();
     const auto& r1 = ers.value<1>();

     BOOST_REQUIRE(r0.is_error());
     BOOST_REQUIRE(r1.is_error());

     auto end = now();
     auto e0  = r0.error();
     auto e1  = r1.error();

     REQUIRE_DURATION(end - start, min(duration0, duration1));
     BOOST_REQUIRE_EQUAL(e0, fault);
     BOOST_REQUIRE_EQUAL(e1, operation_aborted);

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
BOOST_AUTO_TEST_CASE(test_all_wait_and_pause) {
  using masio::wait;
  using masio::pause;

  asio::io_service ios;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto e = pause(ios);

  auto start = now();

  auto p0 = wait(ios, duration0)
         >= [&e]() {
              e.emit();
              return success(now());
            };

  auto p1 = e > wait(ios, duration1)
         >= []() { return success(now()); };

  auto p = all(p0, p1);

  bool executed = false;


  using Results = result<result<Time>, result<Time>>;

  p.execute([&executed, start, duration0, duration1] (Results rs) {
     BOOST_REQUIRE(!rs.is_error());

     BOOST_REQUIRE(rs.value<0>().is_value());
     BOOST_REQUIRE(rs.value<1>().is_value());

     auto end = now();
     auto t0  = rs.value<0>().value<0>();
     auto t1  = rs.value<1>().value<0>();

     REQUIRE_DURATION(t0 - start, duration0);
     REQUIRE_DURATION(t1 - start, duration0 + duration1);
     REQUIRE_DURATION(end - start, duration0 + duration1);

     executed = true;
     });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 3);
}

//--------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_all_wait_and_cancel_subcancelers) {
  using masio::wait;

  asio::io_service ios;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  action<Time> p1;

  auto p0 = wait(ios, duration0)
         >= [&p1]() {
              p1.cancel();
              return success(now());
            }
         >= [](Time t) { return success(t); };

  p1 = wait(ios, duration1) >= []() {
        return success(now());
       }
    >= [](Time t) { return success(t); };

  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = result<result<Time>, result<Time>>;

  p.execute([&executed, start, duration0, duration1]
                  (Results rs) {
     BOOST_REQUIRE(!rs.is_error());

     BOOST_REQUIRE(rs.value<0>().is_value());
     BOOST_REQUIRE(rs.value<1>().is_error());

     auto end = now();
     auto t0  = rs.value<0>().value<0>();
     auto e   = rs.value<1>().error();

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

