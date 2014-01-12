#ifndef __MASIO_POST_H__
#define __MASIO_POST_H__

#include <masio/tools.h>

namespace masio {

struct post {
  using value_type = none_t;

  post(boost::asio::io_service& ios) : _io_service(ios) {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace boost::asio::error;

    _io_service.post([rest, &canceler]() {

        if (canceler.canceled()) {
          rest(typename Error<value_type>::Fail{operation_aborted});
          return;
        }

        rest(typename Error<value_type>::Success{none});
        });
  }

  boost::asio::io_service& _io_service;
};

} // masio namespace
#endif // ifndef __MASIO_POST_H__
