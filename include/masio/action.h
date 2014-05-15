#ifndef __MASIO_ACTION_H__
#define __MASIO_ACTION_H__

namespace masio {

template<typename... A> struct action : monad<A...> {
private:
  typedef std::function<void(result<A...>)> Rest;

  struct wrapper_interface {
    virtual void execute(const Rest&) = 0;
    virtual bool cancel() = 0;
    virtual ~wrapper_interface() {};
  };

  struct State {
    std::shared_ptr<wrapper_interface> delegate;
  };

  template<typename D> struct wrapper : wrapper_interface {
    void execute(const Rest& rest) override {
      delegate.execute(rest);
    }

    bool cancel() { return delegate.cancel(); }

    wrapper(const D& d) : delegate(d) {}
    D delegate;
  };

  template<typename D> struct pointer_wrapper : wrapper_interface {
    void execute(const Rest& rest) override {
      delegate->execute(rest);
    }

    bool cancel() { return delegate->cancel(); }

    pointer_wrapper(const std::shared_ptr<D>& d) : delegate(d) {}
    std::shared_ptr<D> delegate;
  };

public:
  action() : _state(std::make_shared<State>()) { }

  template<typename Delegate>
  action(const Delegate& delegate)
    : _state(std::make_shared<State>())
  {
    _state->delegate = std::make_shared<wrapper<Delegate>>(delegate);
  }

  template<typename Delegate>
  action(const std::shared_ptr<Delegate>& delegate)
    : _state(std::make_shared<State>())
  {
    _state->delegate = std::make_shared<pointer_wrapper<Delegate>>(delegate);
  }

  template<typename Delegate>
  void operator=(const Delegate& delegate) {
    _state->delegate = std::make_shared<wrapper<Delegate>>(delegate);
  }

  template<class Rest>
  void execute(const Rest& rest) {
    _state->delegate->execute(rest);
  }

  bool cancel() {
    return _state->delegate->cancel();
  }

private:
  std::shared_ptr<State> _state;
};

template<class M, class... Args>
typename UseMonadArgs<action, M>::type
make_action(Args&&... args) {
  return typename UseMonadArgs<action, M>::type(std::make_shared<M>(std::forward<Args>(args)...));
}

} // masio namespace

#endif // ifndef __MASIO_ACTION_H__

