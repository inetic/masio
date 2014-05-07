#ifndef __MASIO_ALL_OR_NONE_H__
#define __MASIO_ALL_OR_NONE_H__

namespace masio {

namespace detail {
  template<int I, int Max, class Rest, class Results, class Monads>
  struct AllOrNoneExec {
    static void go( Canceler& canceler
                  , const Monads& monads
                  , const std::shared_ptr<Results>& results
                  , const std::shared_ptr<size_t>& remaining
                  , const std::shared_ptr<boost::system::error_code>& first_error
                  , const Rest& rest) {
      using namespace std;
      using Tuple = Monads;
      using M = typename tuple_element<I, Tuple>::type;
      using R = typename UseMonadArgs<result, M>::type;

      M m = get<I>(monads);

      m.execute(canceler,
           [results, &canceler, rest, first_error, remaining](const R& r) {
         get<I>(results->values()) = r;

         if (r.is_error() && !*first_error) {
           *first_error = r.error();
           canceler.cancel();
         }

         if(--*remaining == 0) {
           rest(typename Results::Success(results->values()));
         }
         });

      detail::AllOrNoneExec<I+1, Max, Rest, Results, Monads>
        ::go(canceler, monads, results, remaining, first_error, rest);
    }
  };

  template<int Max, class Rest, class Results, class Monads>
  struct AllOrNoneExec<Max, Max, Rest, Results, Monads> {
    static void go( Canceler& canceler
                  , const Monads& monads
                  , const std::shared_ptr<Results>& results
                  , const std::shared_ptr<size_t>& remaining
                  , const std::shared_ptr<boost::system::error_code>& first_error
                  , const Rest& rest) {}
  };
} // detail namespace

template<class... Ms> class AllOrNone
  : public monad<typename UseMonadArgs<result, Ms>::type...> {
private:
  using Monads  = std::tuple<Ms...>;
  using Results = result<typename UseMonadArgs<result, Ms>::type...>;

public:
  AllOrNone(const Ms&... monads) : monads(monads...) {}

  template<typename Rest>
  void execute(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using error_code = boost::system::error_code;

    static constexpr unsigned int size = sizeof...(Ms);

    auto results     = make_shared<Results>();
    auto remaining   = make_shared<size_t>(size);
    auto first_error = make_shared<error_code>();

    detail::AllOrNoneExec<0, size, Rest, Results, Monads>
      ::go(canceler, monads, results, remaining, first_error, rest);
  }

private:
  Monads monads;
};

template<typename... Ms>
AllOrNone<Ms...> all_or_none(const Ms&... ms) {
  return AllOrNone<Ms...>(ms...);
}

} // masio namespace

#endif // ifndef __MASIO_ALL_OR_NONE_H__

