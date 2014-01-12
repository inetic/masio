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
  void run(Canceler& s, const Rest& rest) const {
    using namespace boost::asio;

    F fcopy = f;

    ma.run(s, [&s, fcopy, rest](const Error<A>& ea) {
        if (ea.is_error()) {
          rest(typename Error<value_type>::Fail{ea.error()});
        }
        else {
          fcopy(ea.value()).run(s, rest);
        }
        });
  }

  MA ma;
  F  f;
};

} // masio namespace

//template< typename MA
//        , typename F
//        , typename MB = typename std::result_of<F(typename MA::value_type)>::type>
//masio::Bind<MA, MB> operator>=(const MA& ma, const F& f) {
//  return masio::Bind<MA, MB>(ma, f);
//}
//
//template< typename MA
//        , typename MB>
//masio::Bind<MA, MB> operator>(const MA& ma, const MB& mb) {
//  using A = typename MA::value_type;
//  return masio::Bind<MA, MB>(ma, [mb](const A&) { return mb; });
//}

#endif // ifndef __MASIO_BIND_H__
