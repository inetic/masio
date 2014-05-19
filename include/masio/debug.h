#ifndef __MASIO_DEBUG_H__
#define __MASIO_DEBUG_H__

namespace masio {

template<typename MA> class Debug : public UseMonadArgs<monad, MA>::type {
public:
  Debug(const std::string& message, const MA& delegate)
    : _message(message)
    , _delegate(delegate)
  { }

  template<typename Rest> void execute(const Rest& rest) override {
    using namespace boost::asio;

    using OrigResult = typename UseMonadArgs<result, MA>::type;

    std::cout << "started: " << _message << std::endl;

    _delegate.execute([this, rest](const OrigResult& ea) {
        if (ea.is_error()) {
          std::cout << "ended: " << _message
                    << " (" << ea.error().message() << ")" << std::endl;
        }
        else {
          std::cout << "ended: " << _message << std::endl;
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
