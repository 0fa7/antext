#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

// Sends a WebSocket message and prints the response
int main() {

  std::string host = "0.0.0.0";
  std::string port = "8080";

  // The io_context is required for all I/O
  boost::beast::net::io_context ioc;

  // These objects perform our I/O
  boost::asio::ip::tcp::resolver resolver{ioc};
  boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{ioc};

  // Look up the domain name
  auto const results = resolver.resolve(host, port);

  // Make the connection on the IP address we get from a lookup
  auto ep = boost::beast::net::connect(ws.next_layer(), results);

  // Update the host_ string. This will provide the value of the
  // Host HTTP header during the WebSocket handshake.
  // See https://tools.ietf.org/html/rfc7230#section-5.4
  host += ':' + std::to_string(ep.port());

  // Set a decorator to change the User-Agent of the handshake
  ws.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::request_type &req) {
        req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
      }));

  // Perform the websocket handshake
  ws.handshake(host, "/");

  std::string line;
  // This buffer will hold the incoming message
  boost::beast::flat_buffer buffer;

  while (std::getline(std::cin, line)) {
    if (line == "quit") {
      break;
    }

    // Send the message
    ws.write(boost::beast::net::buffer(line));

    // Read a message into our buffer
    ws.read(buffer);

    std::cout << boost::beast::make_printable(buffer.data()) << std::endl;
    buffer.clear();
  }

  // Close the WebSocket connection
  ws.close(boost::beast::websocket::close_code::normal);

  return EXIT_SUCCESS;
}