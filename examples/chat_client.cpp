#include <masio.h>
#include <boost/thread.hpp>
#include <queue>

using namespace std;
using namespace masio;
using namespace boost::asio;
using tcp        = boost::asio::ip::tcp;
using error_code = boost::system::error_code;

struct message {
  enum { header_length   = 4   };
  enum { max_body_length = 512 };

  vector<char> data;

  message() {}

  message(const string& message) {
    data.resize(message.size() + header_length);
    sprintf(&data[0], "%4d", (int) message.size());
    memcpy(&data[4], message.c_str(), message.size());
  }

  string body() {
    return string(data.begin() + header_length, data.end());
  }

  // Header contains only the body length.
  size_t decode_header() {
    char header[message::header_length + 1] = "";
    strncat(header, &data[0], header_length);
    return atoi(header);
  }
};

struct chat_client {
  chat_client(io_service& ios) : socket(ios), block(ios) {}

  ip::tcp::socket socket;
  message         inbound_message;
  queue<message>  outbound_messages;
  masio::pause    block;
};

action<> receive_message(chat_client& c) {
  auto& socket = c.socket;
  auto& m      = c.inbound_message;

  return success()
      >= [&]() { // Receive message header
           m.data.resize(message::header_length);
           return receive(socket, buffer(&m.data[0], message::header_length));
         }
      >= [&]() { // Receive message body
           size_t size = m.decode_header();
           m.data.resize(size + message::header_length);
           return receive(socket, buffer(&m.data[message::header_length], size));
         }
      >= [&]() {
           cout << "Received: " << m.body() << "\n";
           return success();
         };
}

action<> send_message(chat_client& c) {
  using masio::pause;
  using boost::asio::buffer;

  auto& socket = c.socket;
  auto& ms     = c.outbound_messages;

  return success()
      >= [&]() -> action<> {
           if (ms.empty()) {
             return c.block;
           }
           else {
             return send(socket, buffer(ms.front().data))
                 >= [&]() {
                      ms.pop();
                      return success();
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
              >= [&](tcp::resolver::iterator iterator) {
                   return connect(c.socket, iterator);
                 }
              >= [&]() {
                   return all_or_none( forever(receive_message(c))
                                     , forever(send_message(c)))
                        > success();
                 };

  boost::thread thread([&]() {
      program.execute([&](result<> ev) {
        if (ev.is_error()) {
          cerr << "Error " << ev.error().message() << "\n";
        }
        c.socket.close();
        });

      ios.run();

      });

  cout << "Go\n";

  char line[message::max_body_length + 1];

  while(cin.getline(line, message::max_body_length + 1)) {
    string body(line, line + strlen(line));

    ios.post([body, &c]() {
        c.outbound_messages.push(message(body));
        c.block.emit();
        });
  }

  cout << "End\n";
  ios.post([&]() { program.cancel(); });
  thread.join();
}

