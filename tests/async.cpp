#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Sleep
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>

using namespace masio;
using namespace std;
namespace asio = boost::asio;
typedef shared_ptr<State> StatePtr;

BOOST_AUTO_TEST_CASE(test_async) {
  asio::io_service ios;

  StatePtr state = make_shared<State>();

  Cont<int>::Ptr p1 = post<int>(ios, []() { return success<int>(11); });
  Cont<int>::Ptr p2 = post<int>(ios, []() { return success<int>(22); });

  Async<int>::Ptr p = async<int>(p1, p2);

  bool executed = false;

  p->run(state, [&executed](Error<std::vector<Error<int>>> ers) {
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
