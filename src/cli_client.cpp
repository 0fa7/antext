// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket client, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class session;

std::shared_ptr<session> s = nullptr;

void fail(beast::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session> {
public:
  tcp::resolver resolver_;
  websocket::stream<beast::tcp_stream> ws_;
  beast::flat_buffer buffer_;
  std::string host_;
  std::string text_;

  // Resolver and socket require an io_context
  explicit session(net::io_context &ioc)
      : resolver_(net::make_strand(ioc)), ws_(net::make_strand(ioc)) {}

  // Start the asynchronous operation
  void run(char const *host, char const *port) {
    // Save these for later
    host_ = host;

    // Look up the domain name
    resolver_.async_resolve(
        host, port,
        beast::bind_front_handler(&session::on_resolve, shared_from_this()));
  }

  void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec)
      return fail(ec, "resolve");

    // Set the timeout for the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(&session::on_connect, shared_from_this()));
  }

  void on_connect(beast::error_code ec,
                  tcp::resolver::results_type::endpoint_type ep) {
    if (ec)
      return fail(ec, "connect");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(
        websocket::stream_base::decorator([](websocket::request_type &req) {
          req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) +
                      " websocket-client-async");
        }));

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());

    // Perform the websocket handshake
    ws_.async_handshake(
        host_, "/",
        beast::bind_front_handler(&session::on_handshake, shared_from_this()));
  }

  void on_handshake(beast::error_code ec) {
    if (ec)
      return fail(ec, "handshake");

    // Send the message
    /*ws_.async_write(
        net::buffer(text_),
        beast::bind_front_handler(
            &session::on_write,
            shared_from_this()));*/
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    // Read a message into our buffer
    ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read,
                                                      shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "read");

    // Close the WebSocket connection
    ws_.async_close(
        websocket::close_code::normal,
        beast::bind_front_handler(&session::on_close, shared_from_this()));
  }

  void on_close(beast::error_code ec) {
    if (ec)
      return fail(ec, "close");

    // If we get here then the connection is closed gracefully

    // The make_printable() function helps print a ConstBufferSequence
    std::cout << beast::make_printable(buffer_.data()) << std::endl;
  }
};

//------------------------------------------------------------------------------

int main() {
  auto const host = "0.0.0.0";
  auto const port = "8080";
  auto const text = "hello world";

  // The io_context is required for all I/O
  net::io_context ioc;

  s = std::make_shared<session>(ioc);

  ioc.run();

  // Run the I/O service. The call will return when
  // the socket is closed.

  return EXIT_SUCCESS;
}

void CreateSession(std::string host, std::string port) {
  // std::make_shared<session>(ioc)->run(host, port);
}

void CloseConnection() {
  if (s && s->ws_.is_open()) {
    auto reason = websocket::close_code::normal;
    auto res = s->ws_.async_close(reason);
  }
}

void Repl() {
  std::string line;

  while (std::getline(std::cin, line)) {
    if (line == "quit") {
      CloseConnection();
      break;
    }

    
  }
}
