#ifndef __MASIO_BIND_H__
#define __MASIO_BIND_H__

#include <type_traits>

namespace masio {

// [s -> (Ea -> r) -> r] ->
// a -> [s -> (Eb -> r) -> r] ->
// [s -> (Eb -> r) -> r]
template<class MA, class MB> struct Bind
  : UseMonadArgs<monad, MB>::type {

  using F = typename FunctionWithMonadArgs<MB, typename MA::MonadType>::type;

  Bind(const MA& ma, const F& f)
    : ma(ma), f(f) {}

  // [s -> (Ea -> r) -> r]
  template<class Rest>
  void execute(Canceler& s, const Rest& rest) const {
    using namespace boost::asio;
    using ResultA = typename UseMonadArgs<result, MA>::type;
    using ResultB = typename UseMonadArgs<result, MB>::type;

    F fcopy = f;

    ma.execute(s, [&s, fcopy, rest](const ResultA& ea) {
        if (s.canceled()) {
          rest(typename ResultB::Fail{error::operation_aborted});
        }
        else if (ea.is_error()) {
          rest(typename ResultB::Fail{ea.error()});
        }
        else {
          apply_tuple(fcopy, ea.values()).execute(s, rest);
        }
        });
  }

  MA ma;
  F  f;
};

} // masio namespace

#endif // ifndef __MASIO_BIND_H__
