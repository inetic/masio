#ifndef __MASIO_ALL_H__
#define __MASIO_ALL_H__

namespace masio {

namespace detail {
  template<int I, int Max, class Rest, class Results, class Monads>
  struct AllExecute {
    static void go( Monads& monads
                  , const std::shared_ptr<size_t>&  remaining
                  , const std::shared_ptr<Results>& results
                  , const Rest& rest) {
      using namespace std;
      using Tuple = Monads;
      using M = typename tuple_element<I, Tuple>::type;
      using R = typename UseMonadArgs<result, M>::type;

      M& m = get<I>(monads);

      m.execute([remaining, results, rest] (const R& r) {
          get<I>(results->values()) = r;
          if (--*remaining == 0) {
            rest(typename Results::Success(results->values()));
          }
          });

      detail::AllExecute<I+1, Max, Rest, Results, Monads>
        ::go(monads, remaining, results, rest);
    }
  };

  template<int Max, class Rest, class Results, class Monads>
  struct AllExecute<Max, Max, Rest, Results, Monads> {
    static void go( Monads& monads
                  , const std::shared_ptr<size_t>&  remaining
                  , const std::shared_ptr<Results>& results
                  , const Rest& rest) {}
  };
} // detail namespace

template<class... Ms> class All
  : public monad<typename UseMonadArgs<result, Ms>::type...> {
private:
  using Monads  = std::tuple<Ms...>;
  using Results = result<typename UseMonadArgs<result, Ms>::type...>;

public:
  All(const Ms&... monads) : monads(monads...) { }

  template<typename Rest> void execute(const Rest& rest) {
    using namespace std;

    static constexpr unsigned int size = sizeof...(Ms);
    auto remaining = make_shared<size_t>(size);
    auto results   = make_shared<Results>();

    detail::AllExecute<0, size, Rest, Results, Monads>
      ::go(monads, remaining, results, rest);
  }

  bool cancel() {
    return cancel_monad_tuple(monads);
  }

private:
  Monads monads;
};

template<typename... Ms>
All<Ms...> all(const Ms&... ms) {
  return All<Ms...>(ms...);
}

} // masio namespace

#endif // ifndef __MASIO_ALL_H__

