#ifndef __MASIO_RECEIVE_H__
#define __MASIO_RECEIVE_H__

namespace masio {

template<typename MutableBufferSequence>
struct receive_task : monad< receive_task<MutableBufferSequence>
                           , none_t> {
  using tcp = boost::asio::ip::tcp;
  using value_type = none_t;

  receive_task( tcp::socket& socket
              , const MutableBufferSequence& buffer_sequence)
    : socket(socket)
    , buffer_sequence(buffer_sequence)
  {}

  template<class Rest>
  void execute(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename Error<value_type>::Success;
    using Fail    = typename Error<value_type>::Fail;

    auto& s = socket;

    auto cancel_action = make_shared<Canceler::CancelAction>([&s]() {
        s.cancel();
        });

    canceler.link_cancel_action(*cancel_action);

    async_read(socket, buffer_sequence, [rest, &canceler, cancel_action]
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

  tcp::socket&          socket;
  MutableBufferSequence buffer_sequence;
};

template<typename MutableBufferSequence>
receive_task<MutableBufferSequence>
receive( boost::asio::ip::tcp::socket& socket
       , const MutableBufferSequence&  buffer) {
  return receive_task<MutableBufferSequence>(socket, buffer);
}

} // masio namespace
#endif // ifndef __MASIO_RECEIVE_H__
