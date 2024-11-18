#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include "spsc_queue.hpp"
#include "mpmc_waitable_queue.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
std::string host = "localhost";
std::string port = "8080";
std::mutex ws_write_mutex;
std::mutex ws_read_mutex;

mpmc_waitable_queue<int64_t> wq;

int counter = 1;

void input_thread(websocket::stream<tcp::socket> &ws) {

  while (true) {
    std::string line;

    std::getline(std::cin, line);
    {
      std::lock_guard<std::mutex> ws_lock(ws_write_mutex);
      ws.write(net::buffer(line));
      wq.push(counter++);
    }

  }
}

void output_thread(websocket::stream<tcp::socket> &ws) {
  while (true) {
    beast::flat_buffer buffer;
    
    {
      std::lock_guard<std::mutex> ws_lock(ws_read_mutex);
      ws.read(buffer);
      int64_t item;
      wq.wait_pop(item);
      std::cout << "item: " <<  item << std::endl;
    }
    
    std::cout << "recieved: " << beast::make_printable(buffer.data())
              << std::endl;
  }
}

// Sends a WebSocket message and prints the response
int main(int argc, char **argv) {
  try {
    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    websocket::stream<tcp::socket> ws{ioc};

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    auto ep = net::connect(ws.next_layer(), results);

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host += ':' + std::to_string(ep.port());

    // Set a decorator to change the User-Agent of the handshake
    ws.set_option(
        websocket::stream_base::decorator([](websocket::request_type &req) {
          req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) +
                      " websocket-client-coro");
        }));

    // Perform the websocket handshake
    ws.handshake(host, "/");

    std::vector<std::thread> threads;

    threads.push_back(std::thread(input_thread, std::ref(ws)));
    threads.push_back(std::thread(output_thread, std::ref(ws)));

    for (auto &t : threads) {
      t.join();
    }

    // Close the WebSocket connection
    ws.close(websocket::close_code::normal);
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}