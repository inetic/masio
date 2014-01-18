#ifndef __MASIO_ACTION_H__
#define __MASIO_ACTION_H__

namespace masio {

template<typename A> struct action : monad<action<A>, A> {
  using   value_type = A;
  typedef std::function<void(Error<value_type>)> Rest;

  struct wrapper_interface {
    virtual void execute(Canceler&, const Rest&) const = 0;
    virtual ~wrapper_interface() {};
  };

  template<typename D> struct wrapper : wrapper_interface {
    void execute(Canceler& c, const Rest& rest) const override {
      delegate.execute(c, rest);
    }

    wrapper(const D& d) : delegate(d) {}
    D delegate;
  };

  template<typename Delegate>
  action(const Delegate& delegate)
    : _delegate(std::make_shared<wrapper<Delegate>>(delegate))
  {}

  template<class Rest>
  void execute(Canceler& c, const Rest& rest) const {
    _delegate->execute(c, rest);
  }

private:
  std::shared_ptr<wrapper_interface> _delegate;
};

} // masio namespace

#endif // ifndef __MASIO_ACTION_H__

