#ifndef __MAY_FAIL_H__
#define __MAY_FAIL_H__

namespace masio {

template<typename MA> class MayFail {
public:
  using A           = typename MA::value_type;
  using value_type  = Error<A>;

public:
  MayFail(const MA& delegate)
    : _delegate(delegate)
  { }

  template<typename Rest>
  void run(Canceler& c, const Rest& rest) const override {
    using namespace boost::asio;

    _delegate.run(c, [&c, rest](const Error<A>& ea) {
        using Success = typename Error<value_type>::Success;
        using Fail    = typename Error<value_type>::Fail;

        if (c.canceled() && !ea.is_error()) {
          rest(Error<value_type>(Success{Fail{error::operation_aborted}}));
        }
        else {
          rest(Error<value_type>(Success{ea}));
        }
        });
  }

private:
  MA _delegate;
};

template<typename MA> MayFail<MA>
may_fail(const MA& delegate) {
  return MayFail<MA>(delegate);
}

} // masio namespace

#endif // ifndef __MAY_FAIL_H__
