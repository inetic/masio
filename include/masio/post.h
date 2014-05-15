#ifndef __MASIO_POST_H__
#define __MASIO_POST_H__

namespace masio {

struct post : monad<> {
  post(boost::asio::io_service& ios)
    : _io_service(ios)
    , _executing(false)
    , _canceled(false) { }

  template<class Rest> void execute(const Rest& rest) {
    using namespace boost::asio::error;

    _executing = true;
    _canceled  = false;

    _io_service.post([this, rest]() {

        _executing = false;

        if (_canceled) {
          rest(typename result<>::Fail{operation_aborted});
          return;
        }

        rest(typename result<>::Success());
        });
  }

  bool cancel() {
    if (_executing) {
      _canceled = true;
      return true;
    } 
    return false;
  }

private:
  boost::asio::io_service& _io_service;
  bool                     _executing;
  bool                     _canceled;
};

} // masio namespace
#endif // ifndef __MASIO_POST_H__
