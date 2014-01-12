#ifndef __MASIO_WAIT_H__
#define __MASIO_WAIT_H__

namespace masio {

struct wait  {
  using value_type = none_t;

  wait(boost::asio::io_service& ios, unsigned int millis)
    : _io_service(ios)
    , _time(boost::posix_time::milliseconds(millis))
  {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::posix_time;
    using namespace boost::system;

    auto timer = make_shared<deadline_timer>(_io_service, _time);

    auto cancel_action = make_shared<Canceler::CancelAction>([timer]() {
        timer->cancel();
        });

    canceler.link_cancel_action(*cancel_action);

    timer->async_wait([&canceler, rest, timer, cancel_action]
          (const error_code& error){

        cancel_action->unlink();

        if (error) {
          rest(typename Error<value_type>::Fail{error});
        }
        else if (canceler.canceled()) {
          rest(typename Error<value_type>::Fail{error::operation_aborted});
        }
        else {
          rest(typename Error<value_type>::Success{none});
        }
        });
  }

private:
  boost::asio::io_service&         _io_service;
  boost::posix_time::time_duration _time;
};


} // masio namespace

#endif // ifndef __MASIO_WAIT_H__

