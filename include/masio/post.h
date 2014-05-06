#ifndef __MASIO_POST_H__
#define __MASIO_POST_H__

namespace masio {

struct post : monad<none_t> {
  using value_type = none_t;

  post(boost::asio::io_service& ios) : _io_service(ios) {}

  template<class Rest>
  void execute(Canceler& canceler, const Rest& rest) const {
    using namespace boost::asio::error;

    _io_service.post([rest, &canceler]() {

        if (canceler.canceled()) {
          rest(typename result<value_type>::Fail{operation_aborted});
          return;
        }

        rest(typename result<value_type>::Success{none});
        });
  }

  boost::asio::io_service& _io_service;
};

} // masio namespace
#endif // ifndef __MASIO_POST_H__
