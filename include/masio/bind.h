#ifndef __MASIO_BIND_H__
#define __MASIO_BIND_H__

#include <type_traits>
#include "canceler.h"

namespace masio {

// [s -> (Ea -> r) -> r] ->
// a -> [s -> (Eb -> r) -> r] ->
// [s -> (Eb -> r) -> r]
template<class MA, class F> struct Bind {
  typedef std::shared_ptr<Canceler> CancelerPtr;

  using A = typename MA::value_type;
  using value_type = typename std::result_of<F(A)>::type::value_type;

  typedef std::function<void(Error<value_type>)>        Rest;
  typedef std::function<void(const CancelerPtr&, Rest)> Run;

  Bind(const MA& ma, const F& f)
    : ma(ma), f(f) {}

  // [s -> (Ea -> r) -> r]
  void run(const CancelerPtr& s, const Rest& rest) const {
    using namespace boost::asio;

    F fcopy = f; // TODO

    ma.run(s, [s, fcopy, rest](const Error<A>& ea) {
        if (s->canceled()) {
          rest(typename Error<value_type>::Fail{error::operation_aborted});
        }
        else if (ea.is_error()) {
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

template<typename MA, typename F>
masio::Bind<MA, F> operator>=(const MA& ma, const F& f) {
  return masio::Bind<MA, F>(ma, f);
}
#endif // ifndef __MASIO_BIND_H__
