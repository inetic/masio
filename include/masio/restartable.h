#ifndef __MASIO_RESTARTABLE_H__
#define __MASIO_RESTARTABLE_H__

namespace masio {

class restartable : public monad<> {
  typedef std::function<void(result<>)> Rest;

  enum State { idle
             , started
             , canceled
             , canceled_then_started };

  struct Data {
    action<> delegate;
    State    state;
    Rest     rest;

    Data() : state(idle) {}
    Data(const action<>& a) : delegate(a), state(idle) {}
  };

public:
  restartable()                  : _data(std::make_shared<Data>())  {}
  restartable(const action<>& a) : _data(std::make_shared<Data>(a)) {}
  
  restartable& operator=(const action<>& a) {
    set_action(a);
    return *this;
  }

  void set_action(const action<>& a) {
    assert(_data->state == idle);
    _data->delegate = a;
  }

  template<class Rest> void execute(const Rest& rest) {
    _data->rest = rest;
    execute(_data);
  }

  bool cancel() {
    switch (_data->state) {
      case idle: return false;
      case started: _data->state = canceled;
                    _data->delegate.cancel();
                    return true;
      case canceled: return true;
      case canceled_then_started: _data->state = canceled;
                                  return true;
    }
    return false;
  }

  ~restartable() {
    cancel();
  }

private:
  static void execute(const std::shared_ptr<Data>& data) {
    switch (data->state) {
      case idle: data->state = started;
                 data->delegate.execute([data](result<>) {
                       on_executed(data);
                     });
                 break;
      case started: break;
      case canceled: data->state = canceled_then_started; break;
      case canceled_then_started: break;
    }
  }

  static void on_executed(const std::shared_ptr<Data>& data) {
    using Success = result<>::Success;
    using Fail    = result<>::Fail;
    using boost::asio::error::operation_aborted;

    State old_state = data->state;
    data->state = idle;

    switch (old_state) {
      case idle:                                          break;
      case started:  data->rest(Success());               break;
      case canceled: data->rest(Fail{operation_aborted}); break;
      case canceled_then_started: execute(data);          break;
    }
  }

private:
  std::shared_ptr<Data> _data;
};

} // masio namespace

#endif // ifndef __MASIO_RESTARTABLE_H__
