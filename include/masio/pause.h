#ifndef __MASIO_PAUSE_H__
#define __MASIO_PAUSE_H__

namespace masio {

class pause;

class pause : public monad<> {
private:
  typedef std::function<void(result<>)> Rest;
  typedef boost::asio::io_service::work Work;
  typedef result<>::Success Success;
  typedef result<>::Fail    Fail;

  struct State {
    std::shared_ptr<Work> work;
    bool                  canceled;
    Rest                  rest;
  };

public:
  pause(boost::asio::io_service& io_service)
    : io_service(io_service)
    , state(std::make_shared<State>())
  {
    state->canceled  = false;
  }

  template<class Rest>
  void execute(const Rest& rest) {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;

    state->rest = rest;
    state->work = make_shared<Work>(io_service);
  }

  // Return true if there was a pending action.
  bool emit() {
    using namespace std;
    using namespace boost::asio::error;

    if (!state->rest) { return false; }
    if (!state->work) {
      // Either emit() or cancel() was previously called.
      return true;
    }

    state->work = nullptr;

    io_service.post([this]() {
        if (state->canceled) {
          state->canceled = false;
          run_rest(Fail{operation_aborted});
        }
        else {
          run_rest(Success());
        }
        });

    return true;
  }

  bool cancel() {
    using namespace boost::asio::error;
    if (!state->rest) { return false; }

    state->canceled = true;

    if (state->work) {
      // Neither emit() nor cancel() was previously called.
      state->work     = nullptr;
      io_service.post([this]() {
          run_rest(Fail{operation_aborted});
          });
    }

    return true;
  }

private:
  template<class Result> void run_rest(const Result& result) {
    auto rest = state->rest;
    state->rest = nullptr;
    rest(result);
  }

private:
  boost::asio::io_service& io_service;
  std::shared_ptr<State>   state;
};

} // masio namespace
#endif // ifndef __MASIO_PAUSE_H__
