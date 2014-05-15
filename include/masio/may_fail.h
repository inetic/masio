#ifndef __MAY_FAIL_H__
#define __MAY_FAIL_H__

namespace masio {

template<typename MA> class MayFail
  : public monad<typename UseMonadArgs<result, MA>::type> {
public:
  MayFail(const MA& delegate)
    : _delegate(delegate)
  { }

  template<typename Rest> void execute(const Rest& rest) override {
    using namespace boost::asio;

    using OrigResult = typename UseMonadArgs<result, MA>::type;
    using Result     = result<OrigResult>;

    _delegate.execute([rest](const OrigResult& ea) {
        using Success = typename Result::Success;
        //using Fail    = typename Result::Fail;

        rest(Result(Success{ea}));
        //if (c.canceled() && !ea.is_error()) {
        //  rest(Result(Success{Fail{error::operation_aborted}}));
        //}
        //else {
        //  rest(Result(Success{ea}));
        //}
      });
  }

  bool cancel() {
    return _delegate.cancel();
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
