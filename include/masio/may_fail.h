#ifndef __MAY_FAIL_H__
#define __MAY_FAIL_H__

namespace masio {

template<typename MA> class MayFail
  : public monad<result<typename MA::value_type>> {
public:
  using A = typename MA::value_type;

public:
  MayFail(const MA& delegate)
    : _delegate(delegate)
  { }

  template<typename Rest>
  void execute(Canceler& c, const Rest& rest) const override {
    using namespace boost::asio;
    using value_type = result<A>;

    _delegate.execute(c, [&c, rest](const result<A>& ea) {
        using Success = typename result<value_type>::Success;
        using Fail    = typename result<value_type>::Fail;

        if (c.canceled() && !ea.is_error()) {
          rest(result<value_type>(Success{Fail{error::operation_aborted}}));
        }
        else {
          rest(result<value_type>(Success{ea}));
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
