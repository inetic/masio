#ifndef __MASIO_POST_H__
#define __MASIO_POST_H__

#include <masio/tools.h>

namespace masio {

template<class A> struct Post : public Cont<A> {
  typedef Lambda<A>            Super;
  typedef typename Super::Rest Rest;
  typedef typename Super::Run  Run;

  Post(boost::asio::io_service& ios, Run r)
    : _run(r)
    , _io_service(ios) {}

  void run(const Rest& rest) const {
    auto self = this->shared_from_this();
    _io_service.post([=]() { capture(self); _run(rest); });
  }

  Run _run;
  boost::asio::io_service& _io_service;
};

template<class A>
std::shared_ptr<Post<A>> post( boost::asio::io_service& ios
                             , const typename Cont<A>::Run& run) {
  return std::make_shared<Post<A>>(ios, run);
}

} // masio namespace
#endif // ifndef __MASIO_POST_H__
