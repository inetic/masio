#include <masio.h>
#include <boost/thread.hpp>
#include <queue>

using namespace std;
using namespace masio;
using tcp = boost::asio::ip::tcp;
using namespace boost::asio;
using error_code = boost::system::error_code;

struct message {
  enum { header_length   = 4   };
  enum { max_body_length = 512 };

  vector<char> data;

  void set(const string& message) {
    data.resize(message.size() + header_length);
    sprintf(&data[0], "%4d", message.size());
    memcpy(&data[4], message.c_str(), message.size());
  }

  std::string body() {
    return std::string(data.begin() + header_length, data.end());
  }

  // Header only contains the body length.
  size_t decode_header() {
    char header[message::header_length + 1] = "";
    strncat(header, &data[0], header_length);
    return std::atoi(header);
  }
};

struct chat_client {
  chat_client(io_service& ios) : socket(ios) {}

  ip::tcp::socket socket;
  message         inbound_message;
  queue<message>  outbound_messages;
  kicker          kick;
};

action<none_t> receive_message(chat_client& c) {
  auto& socket = c.socket;
  auto& m      = c.inbound_message;

  return success(none)
      >= [&m, &socket](none_t) { // Receive message header
           m.data.resize(message::header_length);
           return receive(socket, buffer(&m.data[0], message::header_length));
         }
      >= [&m, &socket](none_t) { // Receive message body
           size_t size = m.decode_header();
           m.data.resize(size + message::header_length);
           return receive(socket, buffer(&m.data[message::header_length], size));
         }
      >= [&m](none_t) {
           std::cout << "Received: " << m.body() << "\n";
           return success(none);
         }; 
}

action<none_t> send_message(chat_client& c) {
  using masio::pause;
  using boost::asio::buffer;

  auto& socket = c.socket;
  auto& ms     = c.outbound_messages;
  auto& ios    = socket.get_io_service();

  return success(none)
      >= [&socket, &c, &ms, &ios](none_t) -> action<none_t> {
           if (ms.empty()) {
             return pause(ios, c.kick);
           }
           else {
             return send(socket, buffer(ms.front().data))
                 >= [&ms](none_t) {
                      ms.pop();
                      return success(none);
                    };
           }
         };
}

int main(int argc, char* argv[]) {

  if (argc != 3) {
    cerr << "Usage: chat_client <host> <port>\n";
    return 1;
  }

  io_service ios;
  chat_client c(ios);

  auto program = resolve(ios, argv[1], argv[2])
              >= [&c](tcp::resolver::iterator iterator) {
                   return connect(c.socket, iterator);
                 }
              >= [&c](none_t) {
                   return all( forever(receive_message(c))
                             , forever(send_message(c)))
                            > success(none);
                 };

  Canceler canceler;

  boost::thread thread([&ios, &program, &c, &canceler]() {

      program.run(canceler, [&c](Error<none_t> ev) {
        if (ev.is_error()) {
          cout << "Error " << ev.error().message() << "\n";
        }
        c.socket.close();
        });

      ios.run();

      cout << "Thread ended.\n";
      });

  cout << "Go\n";

  char line[message::max_body_length + 1];

  while(std::cin.getline(line, message::max_body_length + 1)) {
    std::string body(line, line + std::strlen(line));
    ios.post([body, &c]() {
        message msg;
        msg.set(body);
        c.outbound_messages.push(msg);
        c.kick();
        });
  }

  thread.join();
}

