#ifndef __MASIO_RETURN_H__
#define __MASIO_RETURN_H__

namespace masio {

template<class... A> struct Return : monad<A...> {
  Return(const A&... a) : value(a...) {}

  template<typename Rest>
  void execute(const Rest& rest) const {
    rest(typename result<A...>::Success(value));
  }

  // Not asynchronous, so not cancelable.
  bool cancel() { return false; }

  std::tuple<A...> value;
};

template<class... A> Return<A...> success(const A&... a) {
  return Return<A...>(a...);
}

} // masio namespace

#endif // ifndef __MASIO_RETURN_H__
