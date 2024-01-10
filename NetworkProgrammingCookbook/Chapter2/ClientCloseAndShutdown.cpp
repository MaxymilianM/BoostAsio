#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

// g++ -ansi -std=c++11 ClientCloseAndShutdown.cpp -o ClientCloseAndShutdown -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread -lboost_chrono

/*
The purpose of the client application is to allocate the socket and connect it to the server application.
After the connection is established, the application should prepare and send a request message
notifying its boundary by shutting down the socket after writing the message to it.

After the request is sent, the client application should read the response.
The size of the response is unknown;
therefore, the reading should be performed until the server closes its socket to notify the response boundary.
*/

// Accepts a reference to the socket object connected to the server
// and performs the communication with the server using this socket.
void communicate(asio::ip::tcp::socket& sock) {
  // Allocating and filling the buffer with binary data.
  const char request_buf[] = {0x48, 0x65, 0x0, 0x6c, 0x6c, 0x6f};

  // Sending the request data.
  asio::write(sock, asio::buffer(request_buf));

  // Shutting down the socket to let the
  // server know that we've sent the whole
  // request.
  sock.shutdown(asio::socket_base::shutdown_send);

  // The client application writes a request message to the socket and then calls the socket's shutdown() method,
  // passing an asio::socket_base::shutdown_send constant as an argument.
  // This call shuts down the send part of the socket.
  // At this point, writing to the socket is disabled,
  // and there is no way to restore the socket state to make it writable again.

  // Shutting down the socket in the client application is seen in the server application
  // as a protocol service message that arrives to the server,
  // notifying the fact that the peer application has shut down the socket.
  // Boost.Asio delivers this message to the application code
  // by means of an error code returned by the asio::read() function.
  // The Boost.Asio library defines this code as asio::error::eof.

  // We use extensible buffer for response
  // because we don't know the size of the
  // response message.
  asio::streambuf response_buf;

  system::error_code ec;
  asio::read(sock, response_buf, ec);

  // std::string str(response_buf.data().begin(), response_buf.data().end());
  // std::cout << "Request: " << str << std::endl;

  // Meanwhile, the client application is blocked in the asio::read() function
  // and reads the response sent by the server until the function returns
  // with the error code equal to asio::error::eof,
  // which implies that the server has finished sending the response message.
  // When the asio::read() function returns with this error code,
  // the client knows that it has read the whole response message, and it can then start processing it
  if (ec == asio::error::eof) {
    // Whole response message has been received.
    // Here we can handle it.
  }
  else {
    throw system::system_error(ec);
  }
}

int main()
{
  std::string raw_ip_address = "127.0.0.1";
  unsigned short port_num = 3333;

  try {
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address),
                               port_num);

    asio::io_service ios;

    asio::ip::tcp::socket sock(ios, ep.protocol());

    sock.connect(ep);

    communicate(sock);
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  // In order to close an allocated socket, the close() method should be called on the corresponding object of the asio::ip::tcp::socket class.
  // However, usually, there is no need to do it explicitly because the destructor of the socket object "~socket()"" closes the socket
  // if one was not closed explicitly.

  return 0;
}
