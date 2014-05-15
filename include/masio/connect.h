#ifndef __MASIO_CONNECT_H__
#define __MASIO_CONNECT_H__

namespace masio {

template<typename Iterator> struct connect_task : monad<> {
  using tcp = boost::asio::ip::tcp;

  connect_task(tcp::socket& socket, Iterator iterator)
    : socket(socket)
    , iterator(iterator)
  {}

  template<class Rest>
  void execute(const Rest& rest) {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename result<>::Success;
    using Fail    = typename result<>::Fail;

    canceled = std::make_shared<bool>(false);

    async_connect(socket, iterator, [this, rest]
        (error_code error, Iterator /* i TODO */) {

        bool c = *canceled;
        canceled = nullptr;

        if (c) {
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
    if (!canceled) return false;
    if (*canceled) return true;
    *canceled = true;
    socket.close();
    return true;
  }

private:
  tcp::socket&          socket;
  Iterator              iterator;
  std::shared_ptr<bool> canceled;
};

template<typename Iterator>
connect_task<Iterator> connect( boost::asio::ip::tcp::socket& socket
                              , Iterator iterator) {
  return connect_task<Iterator>(socket, iterator);
}

} // masio namespace
#endif // ifndef __MASIO_CONNECT_H__
