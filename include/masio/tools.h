#ifndef __MASIO_TOOLS_H__
#define __MASIO_TOOLS_H__

namespace masio {

// To explicityly capture some variables inside a lambda
// function as it seems a bug in g++ prohibits this when
// [=] is used.
//
// Example:
//   void run_buggy(const Rest& rest) const {
//     auto self = this->shared_from_this();
//     _io_service.post([=, self]() { _run(rest); });
//   }
//
//   void run_ok(const Rest& rest) const {
//     auto self = this->shared_from_this();
//     _io_service.post([=]() { capture(self); _run(rest); });
//   }
template<class A> void capture(const A&) {}

////////////////////////////////////////////////////////////////////////////////
// Apply tuple
namespace helper
{
  template <int... Is>
  struct index {};

  template <int N, int... Is>
  struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

  template <int... Is>
  struct gen_seq<0, Is...> : index<Is...> {};
}

template <class F, typename... Args, int... Is>
inline constexpr auto
apply_tuple(const F& f, const std::tuple<Args...>& tup, helper::index<Is...>)
  -> decltype(f(std::get<Is>(tup)...))
{
  return f(std::get<Is>(tup)...);
}

template <class F, typename... Args>
inline constexpr auto
apply_tuple(const F& f, const std::tuple<Args...>& tup)
  -> decltype(apply_tuple(f, tup, helper::gen_seq<sizeof...(Args)>{}))
{
  return apply_tuple(f, tup, helper::gen_seq<sizeof...(Args)>{});
}

////////////////////////////////////////////////////////////////////////////////
// ResultOf
template<class F, class Monad> struct ResultOf {};

template<class F, template<typename...> class Monad, typename... Args>
struct ResultOf<F, Monad<Args...>> {
  typedef typename std::result_of<F(Args...)>::type type;
};

////////////////////////////////////////////////////////////////////////////////
// UseMonadArgs
template<template<typename...> class Template, class Monad>
struct UseMonadArgsImpl {};

template< template<typename...> class Template
        , template<typename...> class Monad
        , typename... Args>
struct UseMonadArgsImpl<Template, Monad<Args...>> {
  typedef Template<Args...> type;
};

template<template<typename...> class Template, class Monad>
struct UseMonadArgs {
  typedef typename UseMonadArgsImpl< Template
                                   , typename Monad::MonadType>::type type;
};

////////////////////////////////////////////////////////////////////////////////
// FunctionWithMonadArgs
template<class Return, class Monad> struct FunctionWithMonadArgs {};

template< class Return
        , template<typename...> class Monad
        , typename... Args>
struct FunctionWithMonadArgs<Return, Monad<Args...>> {
  typedef std::function<Return(Args...)> type;
};

////////////////////////////////////////////////////////////////////////////////
// drop_args

template<class F> struct drop_args_t {
  F f;
  drop_args_t(const F& f) : f(f) {}
  template<typename... Args> 
  typename std::result_of<F()>::type operator()(const Args&...) const {
    return f();
  }
};

template<class F> drop_args_t<F> drop_args(const F& f) {
  return drop_args_t<F>(f);
}

////////////////////////////////////////////////////////////////////////////////
// insert_code
template<class C, class F> struct insert_code_t {
  C c;
  F f;
  insert_code_t(const C& c, const F& f) : c(c), f(f) {}

  template<typename... Args> 
  typename std::result_of<F(Args...)>::type
  operator()(const Args&... args) const {
    c();
    return f(args...);
  }
};

template<class C, class F>
insert_code_t<C, F> insert_code(const C& c, const F& f) {
  return insert_code_t<C, F>(c, f);
}

////////////////////////////////////////////////////////////////////////////////
// cancel_monad_tuple
namespace detail {
  template<int I, int Max, class Monads>
  struct CancelMonadTuple {
    static bool go(Monads& monads) {
      bool result = std::get<I>(monads).cancel();
      return result
          || detail::CancelMonadTuple<I+1, Max, Monads>::go(monads);
    }
  };

  template<int Max, class Monads>
  struct CancelMonadTuple<Max, Max, Monads> {
    static bool go(Monads& monads) { return false; }
  };
} // detail namespace

template<class MonadTuple>
bool cancel_monad_tuple(MonadTuple& monads) {
    return detail::CancelMonadTuple< 0
                                   , std::tuple_size<MonadTuple>::value
                                   , MonadTuple
                                   >::go(monads);
}

} // masio namespace

#endif // ifndef __MASIO_TOOLS_H__
