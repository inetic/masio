#ifndef __MASIO_CONT_H__
#define __MASIO_CONT_H__

namespace masio {

template<class> struct Lambda;

template<class A> struct Cont : public std::enable_shared_from_this<Cont<A>> {
  typedef std::function<void(Error<A>)>  Rest;
  typedef std::function<void(Rest)>      Run;
  typedef std::shared_ptr<Cont<A>>       Ptr;

  virtual void run(const Rest& rest) const = 0;

  template<class B>
  typename Cont<B>::Ptr bind(std::function<typename Cont<B>::Ptr (A)> f) {
    using namespace std;
    typedef typename Cont<B>::Rest BRest;

    auto self = this->shared_from_this();

    return make_shared<Lambda<B>>(([self, f](BRest brest) /* -> void */ {
        self->run([brest,f, self](Error<A> ea) {
            if (!ea.is_error()) {
              f(ea.value())->run(brest);
            }
            else {
              brest(Error<B>(ea.error()));
            }
          });
        }));
  }
};

} // masio namespace

#endif // ifndef __MASIO_CONT_H__
