#ifndef __MASIO_RESOLVE_H__
#define __MASIO_RESOLVE_H__

namespace masio {

struct resolve : monad<resolve, boost::asio::ip::tcp::resolver::iterator> {
  using tcp = boost::asio::ip::tcp;
  using value_type = tcp::resolver::iterator;

  resolve( boost::asio::io_service& io_service
         , const std::string& host
         , const std::string& port)
    : io_service(io_service)
    , host(host)
    , port(port)
  {}

  resolve( boost::asio::io_service& io_service
         , const std::string& host
         , unsigned short port)
    : io_service(io_service)
    , host(host)
    , port(std::to_string(port))
  {}

  template<class Rest>
  void execute(Canceler& canceler, const Rest& rest) const {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using CancelAction = Canceler::CancelAction;
    using Success = typename Error<value_type>::Success;
    using Fail    = typename Error<value_type>::Fail;

    auto resolver = make_shared<tcp::resolver>(io_service);

    auto cancel_action = make_shared<CancelAction>([resolver]() {
        resolver->cancel();
        });

    canceler.link_cancel_action(*cancel_action);

    tcp::resolver::query query(host, port);

    resolver->async_resolve(query, [rest, &canceler, cancel_action]
        (error_code error, tcp::resolver::iterator iterator) {

        cancel_action->unlink();

        if (canceler.canceled()) {
          rest(Fail{operation_aborted});
        }
        else if (error) {
          rest(Fail{error});
        }
        else {
          rest(Success{iterator});
        }
        });
  }

  boost::asio::io_service& io_service;
  std::string              host;
  std::string              port;
};

} // masio namespace
#endif // ifndef __MASIO_RESOLVE_H__
