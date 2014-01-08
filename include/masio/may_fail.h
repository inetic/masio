#ifndef __MAY_FAIL_H__
#define __MAY_FAIL_H__

namespace masio {

template<class A>
class MayFail : public Task<Error<A>> {
public:
  typedef Task<Error<A>>              Super;
  typedef typename Super::CancelerPtr CancelerPtr;
  typedef typename Super::Rest        Rest;
  typedef typename Super::Run         Run;

public:
  MayFail(const std::shared_ptr<Task<A>>& delegate)
    : _delegate(delegate)
  { }

  void run(const CancelerPtr& s, const Rest& rest) const override {
    using namespace boost::asio;

    auto self = Super::shared_from_this();

    _delegate->run(s, [self, s, rest](const Error<A>& ea) {
        typedef typename Error<Error<A>>::Success Success;
        typedef typename Error<Error<A>>::Fail    Fail;

        if (s->canceled() && !ea.is_error()) {
          rest(Error<Error<A>>(Success{Fail{error::operation_aborted}}));
        }
        else {
          rest(Error<Error<A>>(Success{ea}));
        }
        });
  }

private:
  std::shared_ptr<Task<A>> _delegate;
};

template<class A> std::shared_ptr<MayFail<A>>
may_fail(const std::shared_ptr<Task<A>>& delegate) {
  return std::make_shared<MayFail<A>>(delegate);
}

} // masio namespace

#endif // ifndef __MAY_FAIL_H__
