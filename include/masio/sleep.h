#ifndef __MASIO_SLEEP_H__
#define __MASIO_SLEEP_H__

namespace masio {

template<class H, class A> struct Sleep  {
  using CancelerPtr = std::shared_ptr<Canceler>;
  using value_type  = A;

  Sleep(boost::asio::io_service& ios, unsigned int millis, const H& h)
    : _handler(h)
    , _io_service(ios)
    , _time(boost::posix_time::milliseconds(millis))
  {}

  template<class Rest>
  void run(const CancelerPtr& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::posix_time;
    using namespace boost::system;

    auto timer = make_shared<deadline_timer>(_io_service, _time);

    auto cancel_action = make_shared<Canceler::CancelAction>([timer]() {
        timer->cancel();
        });

    canceler->link_cancel_action(*cancel_action);

    auto h = _handler;

    timer->async_wait([h, canceler, rest, timer, cancel_action]
          (const error_code& error){

        cancel_action->unlink();

        if (error) {
          rest(typename Error<A>::Fail{error});
        }
        else if (canceler->canceled()) {
          rest(typename Error<A>::Fail{error::operation_aborted});
        }
        else {
          h().run(canceler, rest);
        }
        });
  }

private:
  H                                _handler;
  boost::asio::io_service&         _io_service;
  boost::posix_time::time_duration _time;
};

//------------------------------------------------------------------------------

template< typename F
        , typename A = typename std::result_of<F()>::type::value_type>
Sleep<F, A>
sleep( boost::asio::io_service& ios
     , unsigned int millis
     , const F& handler) {
  return Sleep<F, A>(ios, millis, handler);
}

//------------------------------------------------------------------------------

} // masio namespace

#endif // ifndef __MASIO_SLEEP_H__

