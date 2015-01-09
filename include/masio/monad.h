#ifndef __MASIO_MONAD_H__
#define __MASIO_MONAD_H__

#include <type_traits>

namespace masio {

template<class, class> struct Bind;

template<typename... A>
struct monad {
  typedef monad<A...> MonadType;
  static const bool is_monad = true;
};

template< class MA
        , typename F
        , typename MB = typename ResultOf<F, typename MA::MonadType>::type
        // Only apply these operators if MA and MB are monads.
        , class = typename std::enable_if<MA::is_monad>::type
        , class = typename std::enable_if<MB::is_monad>::type
        >
Bind<MA, MB> operator>=(const MA& ma, const F& f) {
  return Bind<MA, MB>(ma, f);
}

template< class MA
        , typename F
        , typename MB = typename ResultOf<F, typename MA::MonadType>::type
        // Only apply these operators if MA and MB are monads.
        , class = typename std::enable_if<MA::is_monad>::type
        , class = typename std::enable_if<MB::is_monad>::type
        >
Bind<MA, MB> operator>=(MA&& ma, const F& f) {
  return Bind<MA, MB>(std::forward<MA>(ma), f);
}

template< typename MA
        , typename MB
        // Only apply these operators if MA and MB are monads.
        , class = typename std::enable_if<MA::is_monad>::type
        , class = typename std::enable_if<MB::is_monad>::type>
Bind<MA, MB> operator>(const MA& ma, const MB& mb) {
  return Bind<MA, MB>(ma, drop_args([mb]() { return mb; }));
}

} // masio namespace
#endif // ifndef __MASIO_MONAD_H__
