#ifndef __MASIO_DEBUG_H__
#define __MASIO_DEBUG_H__

#ifdef __ANDROID__
#include <android/log.h>
#else
#include <iostream>
#endif // ifdef __ANDROID__

namespace masio {

template<typename MA> class Debug : public UseMonadArgs<monad, MA>::type {
public:
  Debug(const std::string& message, const MA& delegate)
    : _message(message)
    , _delegate(delegate)
  { }

  template<typename Rest> void execute(const Rest& rest) {
    using namespace boost::asio;

    using OrigResult = typename UseMonadArgs<result, MA>::type;

// TODO: Deuglize this mess
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_DEBUG, "masio"
                       , "started: %s", _message.c_str());
#else
    std::cout << "started: " << _message << std::endl;
#endif

    _delegate.execute([this, rest](const OrigResult& ea) {
        if (ea.is_error()) {
#ifdef __ANDROID__
          __android_log_print(ANDROID_LOG_DEBUG, "masio"
                             , "ended: %s (%s)"
                             , _message.c_str()
                             , ea.error().message().c_str());
#else
          std::cout << "ended: " << _message
                    << " (" << ea.error().message() << ")" << std::endl;
#endif
        }
        else {
#ifndef __ANDROID__
          std::cout << "ended: " << _message << std::endl;
#else
          __android_log_print(ANDROID_LOG_DEBUG, "masio"
                             , "ended: %s", _message.c_str());
#endif
        }
        rest(ea);
      });
  }

  bool cancel() {
    return _delegate.cancel();
  }

private:
  std::string _message;
  MA          _delegate;
};

template<typename MA> Debug<MA>
debug(const std::string& message, const MA& delegate) {
  return Debug<MA>(message, delegate);
}

} // masio namespace

#endif // ifndef __MASIO_DEBUG_H__
