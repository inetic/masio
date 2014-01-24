#ifndef __MASIO_BIND_H__
#define __MASIO_BIND_H__

#include <type_traits>

namespace masio {

// [s -> (Ea -> r) -> r] ->
// a -> [s -> (Eb -> r) -> r] ->
// [s -> (Eb -> r) -> r]
template<class MA, class MB> struct Bind 
    : monad<Bind<MA, MB>, typename MB::value_type> {

  using A = typename MA::value_type;
  using F = std::function<MB(A)>;
  using value_type = typename MB::value_type;

  Bind(const MA& ma, const F& f)
    : ma(ma), f(f) {}

  // [s -> (Ea -> r) -> r]
  template<class Rest>
  void execute(Canceler& s, const Rest& rest) const {
    using namespace boost::asio;

    F fcopy = f;

    ma.execute(s, [&s, fcopy, rest](const result<A>& ea) {
        if (ea.is_error()) {
          rest(typename result<value_type>::Fail{ea.error()});
        }
        else {
          fcopy(ea.value()).execute(s, rest);
        }
        });
  }

  MA ma;
  F  f;
};

} // masio namespace

#endif // ifndef __MASIO_BIND_H__
