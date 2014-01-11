#ifndef __MASIO_TASK_H__
#define __MASIO_TASK_H__

namespace masio {

template<typename A> struct task {
  using   value_type = A;
  typedef std::function<void(Error<value_type>)> Rest;
  typedef std::shared_ptr<Canceler> CancelerPtr;

  struct wrapper_interface {
    virtual void run(const CancelerPtr&, const Rest&) const = 0;
    virtual ~wrapper_interface() {};
  };

  template<typename D> struct wrapper : wrapper_interface {
    void run(const CancelerPtr& c, const Rest& rest) const override {
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
  void run(const CancelerPtr& s, const Rest& rest) const {
    _delegate->run(s, rest);
  }

private:
  std::shared_ptr<wrapper_interface> _delegate;
};

} // masio namespace

#endif // ifndef __MASIO_TASK_H__

