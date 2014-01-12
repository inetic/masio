#ifndef __MASIO_CONNECT_H__
#define __MASIO_CONNECT_H__

namespace masio {

template<typename Iterator> struct connect_task
  : monad<connect_task<Iterator>, none_t> {
  using tcp = boost::asio::ip::tcp;
  using value_type = none_t;

  connect_task(tcp::socket& socket, Iterator iterator)
    : socket(socket)
    , iterator(iterator)
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

    async_connect(socket, iterator, [rest, &canceler, cancel_action]
        (error_code error, Iterator /* i TODO */) {

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

  tcp::socket& socket;
  Iterator     iterator;
};

template<typename Iterator>
connect_task<Iterator> connect( boost::asio::ip::tcp::socket& socket
                              , Iterator iterator) {
  return connect_task<Iterator>(socket, iterator);
}

} // masio namespace
#endif // ifndef __MASIO_CONNECT_H__
