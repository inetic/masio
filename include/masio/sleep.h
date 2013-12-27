#ifndef __MASIO_SLEEP_H__
#define __MASIO_SLEEP_H__

namespace masio {

template<class A>
struct Sleep : public masio::Cont<A> {
  typedef typename Cont<A>::StatePtr StatePtr;
  typedef typename Cont<A>::Rest     Rest;
  typedef typename Cont<A>::Run      Run;

  typedef std::function<typename Cont<A>::Ptr ()> Handler;

  Sleep(boost::asio::io_service& ios, unsigned int millis, const Handler& r)
    : _handler(r)
    , _io_service(ios)
    , _time(boost::posix_time::milliseconds(millis))
  {}

  void run(const StatePtr& state, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::posix_time;
    using namespace boost::system;

    auto self = std::static_pointer_cast<const Sleep<A>>
                (Cont<A>::shared_from_this());

    auto timer = make_shared<deadline_timer>(_io_service, _time);

    timer->async_wait([self, state, rest, timer](const error_code& error){

        if (error) {
          rest(Error<A>::make_error(error));
        }
        else if (state->canceled()) {
          rest(Error<A>::make_error(boost::asio::error::operation_aborted));
        }
        else {
          self->_handler()->run(state, rest);
        }
        });
  }

private:
  Handler                     _handler;
  boost::asio::io_service&    _io_service;
  boost::posix_time::time_duration _time;
};

//------------------------------------------------------------------------------

template<class A>
typename Sleep<A>::Ptr sleep( boost::asio::io_service&          ios
                            , unsigned int                      millis
                            , const typename Sleep<A>::Handler& handler) {
  return std::make_shared<Sleep<A>>(ios, millis, handler);
}

//------------------------------------------------------------------------------

} // masio namespace

#endif // ifndef __MASIO_SLEEP_H__

