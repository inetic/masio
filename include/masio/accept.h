#ifndef __MASIO_ACCEPT_H__
#define __MASIO_ACCEPT_H__

namespace masio {

struct Accept : monad<> {
  using tcp = boost::asio::ip::tcp;

  Accept(const Accept&) = delete;
  Accept& operator=(const Accept&) = delete;

  Accept(tcp::socket& socket, unsigned short port)
    : socket(socket)
    , endpoint(tcp::endpoint(tcp::v4(), port))
    , canceled(false)
  {}

  template<class Rest>
  void execute(const Rest& rest) {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename result<>::Success;
    using Fail    = typename result<>::Fail;

    auto& ios = socket.get_io_service();
    acceptor  = make_shared<tcp::acceptor>(ios, endpoint);

    acceptor->async_accept(socket, [this, rest] (error_code error) {
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
    if (!acceptor) { return false; }
    if (canceled) { return true; }
    canceled = true;
    acceptor->cancel();
    return true;
  }

private:
  tcp::socket&                   socket;
  tcp::endpoint                  endpoint;
  std::shared_ptr<tcp::acceptor> acceptor;
  bool                           canceled;
};

inline
action<> accept(boost::asio::ip::tcp::socket& socket, unsigned short port) {
  return make_action<Accept>(socket, port);
}

} // masio namespace
#endif // ifndef __MASIO_ACCEPT_H__
