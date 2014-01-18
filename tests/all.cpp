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

  Canceler canceler;

  auto p1 = post(ios) > success(11);
  auto p2 = post(ios) > success(22);

  auto p = all(p1, p2);

  bool executed = false;

  using Results = std::pair<Error<int>, Error<int>>;

  p.execute(canceler, [&executed](Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_value());

     BOOST_REQUIRE_EQUAL(*rs.first, 11);
     BOOST_REQUIRE_EQUAL(*rs.second, 22);

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

  Canceler canceler;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0) >= [](none_t) { return success(now()); };
  auto p1 = wait(ios, duration1) >= [](none_t) { return success(now()); };

  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = std::pair<Error<Time>, Error<Time>>;

  p.execute(canceler, [&executed, start, duration0, duration1]
                  (Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_value());

     auto end = now();
     auto t0  = *rs.first;
     auto t1  = *rs.second;

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

  Canceler canceler;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0)
         >= [&canceler](none_t) {
              canceler.cancel();
              return success(now());
            };

  auto p1 = wait(ios, duration1)
         >= [](none_t) { return success(now()); };

  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = std::pair<Error<Time>, Error<Time>>;

  p.execute(canceler, [&executed, start, duration0, duration1]
                  (Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_error());

     auto end = now();
     auto t0  = rs.first.value();
     auto e   = rs.second.error();

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

  Canceler canceler;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0)
         >= [&canceler](none_t) { return fail<Time>(fault); };

  auto p1 = wait(ios, duration1)
         >= [](none_t) { return success(now()); };

  auto p = all_or_none(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = std::pair<Error<Time>, Error<Time>>;

  p.execute(canceler, [&executed, start, duration0, duration1]
                  (Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_error());
     BOOST_REQUIRE(rs.second.is_error());

     auto end = now();
     auto e0  = rs.first.error();
     auto e1  = rs.second.error();

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

  Canceler canceler;
  kicker   kick;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0)
         >= [&kick](none_t) {
              kick();
              return success(now());
            };

  auto p1 = pause(ios, kick)
          > wait(ios, duration1)
         >= [](none_t) {
           return success(now());
         };

  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = std::pair<Error<Time>, Error<Time>>;

  p.execute(canceler, [&executed, start, duration0, duration1]
                  (Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_value());

     auto end = now();
     auto t0  = rs.first.value();
     auto t1  = rs.second.value();

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

  Canceler canceler;
  Canceler p1_canceler;

  typedef system_clock::time_point Time;

  unsigned int duration0 = 123;
  unsigned int duration1 = 234;

  auto p0 = wait(ios, duration0)
         >= [&p1_canceler](none_t) {
              p1_canceler.cancel();
              return success(now());
            }
         >= [](Time t) { return success(t); };

  auto p1 = with_canceler( p1_canceler
                         , wait(ios, duration1) >= [](none_t) {
                             return success(now());
                           })
         >= [](Time t) { return success(t); };

  auto p = all(p0, p1);

  bool executed = false;

  auto start = now();

  using Results = std::pair<Error<Time>, Error<Time>>;

  p.execute(canceler, [&executed, start, duration0, duration1]
                  (Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = ers.value();

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_error());

     auto end = now();
     auto t0  = rs.first.value();
     auto e   = rs.second.error();

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

