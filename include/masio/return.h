#ifndef __MASIO_RETURN_H__
#define __MASIO_RETURN_H__

namespace masio {

template<class A> struct Return {
  typedef std::shared_ptr<Canceler>   CancelerPtr;

  using value_type = A;

  Return(const A& a) : value(a) {}

  template<typename Rest>
  void run(const CancelerPtr& canceler, const Rest& rest) const {
    rest(typename Error<A>::Success{value});
  }

  A value;
};

template<class A> Return<A> success(const A& a) {
  return Return<A>(a);
}

} // masio namespace

#endif // ifndef __MASIO_RETURN_H__
