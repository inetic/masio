#ifndef __MASIO_SLEEP_H__
#define __MASIO_SLEEP_H__

namespace masio {

template<class A>
struct Sleep : public masio::Task<A> {
  typedef typename Task<A>::CancelerPtr CancelerPtr;
  typedef typename Task<A>::Rest        Rest;
  typedef typename Task<A>::Run         Run;

  typedef std::function<typename Task<A>::Ptr ()> Handler;

  Sleep(boost::asio::io_service& ios, unsigned int millis, const Handler& r)
    : _handler(r)
    , _io_service(ios)
    , _time(boost::posix_time::milliseconds(millis))
  {}

  void run(const CancelerPtr& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::posix_time;
    using namespace boost::system;

    auto self = static_pointer_cast<const Sleep<A>>
                  (Task<A>::shared_from_this());

    auto timer = make_shared<deadline_timer>(_io_service, _time);

    auto cancel_action = make_shared<Canceler::CancelAction>([timer]() {
        timer->cancel();
        });

    canceler->link_cancel_action(*cancel_action);

    timer->async_wait([self, canceler, rest, timer, cancel_action]
          (const error_code& error){

        cancel_action->unlink();

        if (error) {
          rest(typename Error<A>::Fail{error});
        }
        else if (canceler->canceled()) {
          rest(typename Error<A>::Fail{error::operation_aborted});
        }
        else {
          self->_handler()->run(canceler, rest);
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

