#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Sleep
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>

using namespace masio;
using namespace std;
namespace asio = boost::asio;
typedef shared_ptr<State> StatePtr;

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_sleep) {
  asio::io_service ios;

  StatePtr state = make_shared<State>();

  Cont<int>::Ptr p = post<int>(ios, []() {
    return success<int>(10);
  })
  ->bind<float>([&ios](int a) {
    return sleep<float>(ios, 2*1000, [a]() {
      return success<float>(2*a + 1);
      });
  })
  ->bind<int>([](float a) {
      return success<int>(a+2);
  });

  p->run(state, [](Error<int> i) {
      BOOST_REQUIRE(!i.is_error());
      BOOST_REQUIRE(i.value() == 23);
      });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE_EQUAL(poll_count, 2);

}

