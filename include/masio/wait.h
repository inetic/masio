#ifndef __MASIO_WAIT_H__
#define __MASIO_WAIT_H__

namespace masio {

class wait : public monad<> {
public:
  using error_code = boost::system::error_code;

private:
  using Fail    = result<>::Fail;
  using Success = result<>::Success;

public:
  wait(boost::asio::io_service& ios, unsigned int millis)
    : _io_service(ios)
    , _time(boost::posix_time::milliseconds(millis))
    , _timer(std::make_shared<boost::asio::deadline_timer>(_io_service))
    , _is_executing(std::make_shared<bool>(false))
    , _canceled(std::make_shared<bool>(false))
  {}

  template<class Rest> void execute(const Rest& rest) {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::posix_time;

    *_canceled     = false;
    *_is_executing = true;

    auto is_executing = _is_executing;
    auto canceled     = _canceled;

    auto self = this;
    _timer->expires_from_now(_time);
    _timer->async_wait([is_executing, canceled, rest, self] (const error_code& error) {

        *is_executing = false;

        if (*canceled) {
          *canceled = false;
          rest(Fail{error::operation_aborted});
        }
        else if (error) {
          rest(Fail{error});
        }
        else {
          rest(Success());
        }
      });
  }

  bool cancel() {
    if (!*_is_executing) return false;
    _timer->cancel();
    *_canceled = true;
    return true;
  }

private:
  boost::asio::io_service&                     _io_service;
  boost::posix_time::time_duration             _time;
  std::shared_ptr<boost::asio::deadline_timer> _timer;
  std::shared_ptr<bool>                        _is_executing;
  std::shared_ptr<bool>                        _canceled;
};


} // masio namespace

#endif // ifndef __MASIO_WAIT_H__

