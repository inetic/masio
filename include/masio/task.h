#ifndef __MASIO_TASK_H__
#define __MASIO_TASK_H__

namespace masio {

template<typename A> struct task : monad<task<A>, A> {
  using   value_type = A;
  typedef std::function<void(Error<value_type>)> Rest;

  struct wrapper_interface {
    virtual void run(Canceler&, const Rest&) const = 0;
    virtual ~wrapper_interface() {};
  };

  template<typename D> struct wrapper : wrapper_interface {
    void run(Canceler& c, const Rest& rest) const override {
      delegate.run(c, rest);
    }

    wrapper(const D& d) : delegate(d) {}
    D delegate;
  };

  template<typename Delegate>
  task(const Delegate& delegate)
    : _delegate(std::make_shared<wrapper<Delegate>>(delegate))
  {}

  template<class Rest>
  void run(Canceler& c, const Rest& rest) const {
    _delegate->run(c, rest);
  }

private:
  std::shared_ptr<wrapper_interface> _delegate;
};

} // masio namespace

#endif // ifndef __MASIO_TASK_H__

