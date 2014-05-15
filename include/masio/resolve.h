#ifndef __MASIO_RESOLVE_H__
#define __MASIO_RESOLVE_H__

namespace masio {

struct resolve : monad<boost::asio::ip::tcp::resolver::iterator> {
  using tcp = boost::asio::ip::tcp;
  using value_type = tcp::resolver::iterator;

  resolve( boost::asio::io_service& io_service
         , const std::string& host
         , const std::string& port)
    : io_service(io_service)
    , host(host)
    , port(port)
    , canceled(std::make_shared<bool>(false))
  {
  }

  resolve( boost::asio::io_service& io_service
         , const std::string& host
         , unsigned short port)
    : io_service(io_service)
    , host(host)
    , port(std::to_string(port))
    , canceled(std::make_shared<bool>(false))
  {
  }

  template<class Rest>
  void execute(const Rest& rest) {
    using namespace std;
    using namespace boost::asio;
    using namespace boost::asio::error;
    using boost::system::error_code;
    using Success = typename result<value_type>::Success;
    using Fail    = typename result<value_type>::Fail;

    resolver = make_shared<tcp::resolver>(io_service);

    tcp::resolver::query query(host, port);

    resolver->async_resolve(query, [rest, this]
        (error_code error, tcp::resolver::iterator iterator) {

        resolver = nullptr;

        if (*canceled) {
          *canceled = false;
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

  bool cancel() {
    if (!resolver) return false;
    if (*canceled) return true;
    *canceled = true;
    resolver->cancel();
    return true;
  }

private:
  boost::asio::io_service&       io_service;
  std::string                    host;
  std::string                    port;
  std::shared_ptr<tcp::resolver> resolver;
  std::shared_ptr<bool>          canceled;
};

} // masio namespace
#endif // ifndef __MASIO_RESOLVE_H__
