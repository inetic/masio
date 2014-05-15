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

  Bind(const MA& ma, const F& f) : ma(ma), mb(nullptr), f(f) {}
  Bind(MA&& ma, const F& f)      : ma(ma), mb(nullptr), f(f) {}

  // [s -> (Ea -> r) -> r]
  template<class Rest>
  void execute(const Rest& rest) {
    using namespace boost::asio;
    using ResultA = typename UseMonadArgs<result, MA>::type;
    using ResultB = typename UseMonadArgs<result, MB>::type;

    F fcopy = f;

    ma.execute([fcopy, rest, this](const ResultA& ea) {
        if (ea.is_error()) {
          rest(typename ResultB::Fail{ea.error()});
        }
        else {
          mb = std::make_shared<MB>(apply_tuple(fcopy, ea.values()));
          mb->execute(insert_code([this] () { mb = nullptr; }, rest));
        }
        });
  }

  bool cancel() {
    if (ma.cancel()) {
      return true;
    }
    if (mb && mb->cancel()) {
      return true;
    }
    return false;
  }

private:
  MA  ma;
  std::shared_ptr<MB> mb;
  F   f;
};

} // masio namespace

#endif // ifndef __MASIO_BIND_H__
