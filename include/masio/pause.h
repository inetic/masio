#ifndef __MASIO_PAUSE_H__
#define __MASIO_PAUSE_H__

namespace masio {

class pause;

class kicker {
  friend class pause;
  using Rest = std::function<void ()>;

public:
  void operator()() {
    if (rest) {
      auto copy = rest;
      rest = 0;
      copy();
    }
  }

  kicker() {};
  kicker(const kicker&) = delete;
  kicker& operator=(const kicker&) = delete;

private:
  Rest rest;
};

struct pause : monad<pause, none_t> {
  using value_type = none_t;

  pause(boost::asio::io_service& io_service, kicker& kick)
    : io_service(io_service)
    , kick(kick) {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio::error;
    using Fail = Error<none_t>::Fail;
    using Success = Error<none_t>::Success;

    auto kick_ptr = &kick;
    auto ios_ptr  = &io_service;

    auto cancel_action = make_shared<Canceler::CancelAction>(
        [rest, kick_ptr, ios_ptr]() {
          kick_ptr->rest = 0;
          ios_ptr->post([rest]() { rest(Fail{operation_aborted}); });
        });

    kick.rest = [rest, cancel_action, ios_ptr, &canceler]() {
        cancel_action->unlink();

        ios_ptr->post([rest, &canceler]() {
            if (!canceler.canceled()) {
              rest(Success{none});
            }
            else {
              rest(Fail{operation_aborted});
            }
          });
        };
  }

  boost::asio::io_service& io_service;
  kicker& kick;
};

} // masio namespace
#endif // ifndef __MASIO_PAUSE_H__
