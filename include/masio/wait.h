#ifndef __MASIO_WAIT_H__
#define __MASIO_WAIT_H__

namespace masio {

class wait : public monad<wait, none_t> {
public:
  using value_type = none_t;
  using error_code = boost::system::error_code;

private:
  using Fail    =  Error<value_type>::Fail;
  using Success =  Error<value_type>::Success;

public:

  wait(boost::asio::io_service& ios, unsigned int millis)
    : _io_service(ios)
    , _time(boost::posix_time::milliseconds(millis))
  {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::posix_time;

    if (!canceler.canceled()) {
      auto timer = make_shared<deadline_timer>(_io_service, _time);

      auto cancel_action = make_shared<Canceler::CancelAction>([timer]() {
          timer->cancel();
          });

      canceler.link_cancel_action(*cancel_action);

      timer->async_wait([&canceler, rest, timer, cancel_action]
                        (const error_code& error){
          cancel_action->unlink();

          if (error) {
            rest(Fail{error});
          }
          else if (canceler.canceled()) {
            rest(Fail{error::operation_aborted});
          }
          else {
            rest(Success{none});
          }
        });
    }
    else {
      _io_service.post([rest](){ rest(Fail{error::operation_aborted}); });
    }
  }

private:
  boost::asio::io_service&         _io_service;
  boost::posix_time::time_duration _time;
};


} // masio namespace

#endif // ifndef __MASIO_WAIT_H__

