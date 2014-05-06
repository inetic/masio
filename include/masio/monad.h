#ifndef __MASIO_MONAD_H__
#define __MASIO_MONAD_H__

#include <type_traits>

namespace masio {

template<class, class> class Bind;

template<typename A>
struct monad {
  using value_type = A;
  static const bool is_monad = true;
};

template< typename MA
        , typename F
        , typename MB = typename std::result_of<F(typename MA::value_type)>::type
        // ONly apply these operators if the argument is a monads.
        , class = typename std::enable_if<MA::is_monad>::type
        , class = typename std::enable_if<MB::is_monad>::type>
Bind<MA, MB> operator>=(const MA& ma, const F& f) {
  return Bind<MA, MB>(ma, f);
}
  
template< typename MA
        , typename MB
        // ONly apply these operators if the arguments are monads.
        , class = typename std::enable_if<MA::is_monad>::type
        , class = typename std::enable_if<MB::is_monad>::type>
Bind<MA, MB> operator>(const MA& ma, const MB& mb) {
  using A = typename MA::value_type;
  return Bind<MA, MB>(ma, [mb](const A&) { return mb; });
}

} // masio namespace
#endif // ifndef __MASIO_MONAD_H__
