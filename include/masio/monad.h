#ifndef __MASIO_MONAD_H__
#define __MASIO_MONAD_H__

namespace masio {

template<class, class> class Bind;

template<class MA, typename A>
struct monad {

  using value_type = A;
  
  template< typename F
          , typename MB = typename std::result_of<F(A)>::type>
  Bind<MA, MB> operator>=(const F& f) const {
    return Bind<MA, MB>(static_cast<const MA&>(*this), f);
  }
  
  template< typename MB>
  Bind<MA, MB> operator>(const MB& mb) const {
    return Bind<MA, MB>( static_cast<const MA&>(*this)
                       , [mb](const A&) { return mb; });
  }

};

} // masio namespace
#endif // ifndef __MASIO_MONAD_H__
