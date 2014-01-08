#ifndef __MASIO_POST_H__
#define __MASIO_POST_H__

#include <masio/tools.h>

namespace masio {

template<class A> struct Post : public Task<A> {
  typedef Lambda<A>                   Super;
  typedef typename Super::CancelerPtr CancelerPtr;
  typedef typename Super::Rest        Rest;
  typedef typename Super::Run         Run;
  typedef std::function<typename Task<A>::Ptr ()> Handler;

  Post(boost::asio::io_service& ios, const Handler& r)
    : _handler(r)
    , _io_service(ios) {}

  void run(const CancelerPtr& canceler, const Rest& rest) const {
    using namespace boost::asio::error;

    auto self = this->shared_from_this();

    _io_service.post([=]() {
        capture(self);

        if (canceler->canceled()) {
          rest(typename Error<A>::Fail{operation_aborted});
          return;
        }

        _handler()->run(canceler, rest);
        });
  }

  Handler _handler;
  boost::asio::io_service& _io_service;
};

template<class A>
std::shared_ptr<Post<A>> post( boost::asio::io_service& ios
                             , const typename Post<A>::Handler& handler) {
  return std::make_shared<Post<A>>(ios, handler);
}

} // masio namespace
#endif // ifndef __MASIO_POST_H__
