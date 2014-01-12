#ifndef __MASIO_SEND_H__
#define __MASIO_SEND_H__

namespace masio {

template<typename ConstBufferSequence>
struct send_task : monad<send_task<ConstBufferSequence>, none_t> {
  using tcp = boost::asio::ip::tcp;
  using value_type = none_t;

  send_task(tcp::socket& socket, const ConstBufferSequence& buffer_sequence)
    : socket(socket)
    , buffer_sequence(buffer_sequence)
  {}

  template<class Rest>
  void run(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename Error<value_type>::Success;
    using Fail    = typename Error<value_type>::Fail;

    if (canceler.canceled()) {
      socket.get_io_service().post([rest]() { rest(Fail{operation_aborted});});
      return;
    }

    auto& s = socket;

    auto cancel_action = make_shared<Canceler::CancelAction>([&s]() {
        s.cancel();
        });

    canceler.link_cancel_action(*cancel_action);

    async_write(socket, buffer_sequence, [rest, &canceler, cancel_action]
        (error_code error, size_t /* size */) {

        cancel_action->unlink();

        if (canceler.canceled()) {
          rest(Fail{operation_aborted});
        }
        else if (error) {
          rest(Fail{error});
        }
        else {
          rest(Success{none});
        }
        });
  }

  tcp::socket&        socket;
  ConstBufferSequence buffer_sequence;
};

template<typename ConstBufferSequence>
send_task<ConstBufferSequence> send( boost::asio::ip::tcp::socket& socket
                                   , const ConstBufferSequence&    buffer) {
  return send_task<ConstBufferSequence>(socket, buffer);
}

} // masio namespace
#endif // ifndef __MASIO_SEND_H__
