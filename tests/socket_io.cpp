#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SocketIO
#include <boost/test/unit_test.hpp>

#include <masio.h>
#include <iostream>
#include <chrono>

//------------------------------------------------------------------------------
using namespace masio;
using namespace std;
namespace asio = boost::asio;
using namespace std::chrono;
using tcp = asio::ip::tcp;

//------------------------------------------------------------------------------
system_clock::time_point now() { return system_clock::now(); }

std::ostream& operator<<(std::ostream& os, const system_clock::time_point& t) {
  return os << t.time_since_epoch() / seconds(1);
}

//------------------------------------------------------------------------------
// TODO
tcp::resolver::iterator resolve( boost::asio::io_service& ios
                               , const std::string& host
                               , unsigned short port) {
  tcp::resolver resolver(ios);
  tcp::resolver::query query(host, to_string(port));
  return resolver.resolve(query);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_accept_connect) {
  using masio::accept;

  asio::io_service ios;

  tcp::socket client(ios);
  tcp::socket server(ios);

  unsigned short port = 9090;

  auto p1 = accept(server, port);
  auto p2 = connect(client, resolve(ios, "localhost", port));

  auto p = all(p1, p2);

  bool executed = false;

  using Results = std::pair<Error<none_t>, Error<none_t>>;

  Canceler canceler;

  p.run(canceler, [&executed](Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_value());

     executed = true;
     });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 2);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_connect_accept) {
  using masio::accept;
  using masio::wait;

  asio::io_service ios;

  tcp::socket client(ios);
  tcp::socket server(ios);

  unsigned short port = 9090;

  auto p1 = wait(ios, 100) >> connect(client, resolve(ios, "localhost", port));
  auto p2 = accept(server, port);

  auto p = all(p1, p2);

  bool executed = false;

  using Results = std::pair<Error<none_t>, Error<none_t>>;

  Canceler canceler;

  p.run(canceler, [&executed](Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_value());

     executed = true;
     });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 3);
}

//------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(test_send_receive) {
  using masio::accept;
  using masio::wait;
  using asio::buffer;

  asio::io_service ios;

  tcp::socket client(ios);
  tcp::socket server(ios);

  unsigned short port = 9090;

  std::string tx_buffer = "hello";
  std::string rx_buffer = "XXXXX";

  auto p1 = accept(server, port)
         >> send(server, buffer(tx_buffer, tx_buffer.size()));

  auto p2 = connect(client, resolve(ios, "localhost", port))
         >> receive(client, buffer(&rx_buffer[0], rx_buffer.size()));

  auto p = all(p1, p2);

  bool executed = false;

  using Results = std::pair<Error<none_t>, Error<none_t>>;

  Canceler canceler;

  p.run(canceler, [&executed, &rx_buffer, &tx_buffer](Error<Results> ers) {
     BOOST_REQUIRE(!ers.is_error());

     const Results& rs = *ers;

     BOOST_REQUIRE(rs.first.is_value());
     BOOST_REQUIRE(rs.second.is_value());

     BOOST_REQUIRE_EQUAL(rx_buffer, tx_buffer);

     executed = true;
     });

  int poll_count = 0;

  while(ios.run_one()) {
    ++poll_count;
  }

  BOOST_REQUIRE(executed);
  BOOST_REQUIRE_EQUAL(poll_count, 4);
}

