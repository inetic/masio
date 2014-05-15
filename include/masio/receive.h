#ifndef __MASIO_RECEIVE_H__
#define __MASIO_RECEIVE_H__

namespace masio {

template<typename MutableBufferSequence>
struct Receive : monad<> {
  using tcp = boost::asio::ip::tcp;

  Receive( tcp::socket& socket
         , const MutableBufferSequence& buffer_sequence)
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

    async_read(socket, buffer_sequence, [rest, this]
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
  tcp::socket&          socket;
  MutableBufferSequence buffer_sequence;
  bool                  canceled;
  bool                  running;
};

template<typename MutableBufferSequence>
action<> receive( boost::asio::ip::tcp::socket& socket
                , const MutableBufferSequence&  buffer) {
  return make_action<Receive<MutableBufferSequence>>(socket, buffer);
}

} // masio namespace
#endif // ifndef __MASIO_RECEIVE_H__
