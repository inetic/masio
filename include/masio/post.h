#ifndef __MASIO_POST_H__
#define __MASIO_POST_H__

#include <masio/tools.h>

namespace masio {

template<typename H, class A> struct Post  {
  using value_type = A;

  Post(boost::asio::io_service& ios, const H& h)
    : _handler(h)
    , _io_service(ios) {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace boost::asio::error;

    auto h = _handler;
    _io_service.post([h, rest, &canceler]() {

        if (canceler.canceled()) {
          rest(typename Error<A>::Fail{operation_aborted});
          return;
        }

        h().run(canceler, rest);
        });
  }

  H _handler;
  boost::asio::io_service& _io_service;
};

template< typename F
        , typename A = typename std::result_of<F()>::type::value_type>
Post<F, A>
post(boost::asio::io_service& ios, const F& handler) {
  return Post<F, A>(ios, handler);
}

} // masio namespace
#endif // ifndef __MASIO_POST_H__
