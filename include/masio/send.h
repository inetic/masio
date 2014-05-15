#ifndef __MASIO_SEND_H__
#define __MASIO_SEND_H__

namespace masio {

template<typename ConstBufferSequence>
struct Send : monad<> {
  using tcp = boost::asio::ip::tcp;

  Send(const Send&) = delete;
  Send& operator=(const Send&) = delete;

  Send(tcp::socket& socket, const ConstBufferSequence& buffer_sequence)
    : socket(socket)
    , buffer_sequence(buffer_sequence)
    , canceled(false)
    , running(false)
  {}

  template<class Rest>
  void execute(const Rest& rest) {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename result<>::Success;
    using Fail    = typename result<>::Fail;

    running = true;
    async_write(socket, buffer_sequence, [rest, this]
        (error_code error, size_t /* size */) {
        running = false;

        if (canceled) {
          canceled = false;
          rest(Fail{operation_aborted});
        }
        else if (error) {
          rest(Fail{error});
        }
        else {
          rest(Success());
        }
      });
  }

  bool cancel() {
    if (!running) return false;
    if (canceled) return true;
    canceled = true;
    socket.close();
    return true;
  }

private:
  tcp::socket&        socket;
  ConstBufferSequence buffer_sequence;
  bool                canceled;
  bool                running;
};

template<typename ConstBufferSequence>
action<> send( boost::asio::ip::tcp::socket& socket
                              , const ConstBufferSequence&    buffer) {
  return make_action<Send<ConstBufferSequence>>(socket, buffer);
}

} // masio namespace
#endif // ifndef __MASIO_SEND_H__
