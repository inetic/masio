#ifndef __MASIO_FAIL_H__
#define __MASIO_FAIL_H__

namespace masio {

template<class... A> struct Fail : monad<A...> {
  using error_code = boost::system::error_code;

  Fail(const error_code& error) : error(error) {}

  template<typename Rest>
  void execute(Canceler& canceler, const Rest& rest) const {
    rest(typename result<A...>::Fail{error});
  }

  error_code error;
};

template<class... A>
Fail<A...> fail(const boost::system::error_code& error) {
  return Fail<A...>(error);
}

} // masio namespace

#endif // ifndef __MASIO_FAIL_H__
