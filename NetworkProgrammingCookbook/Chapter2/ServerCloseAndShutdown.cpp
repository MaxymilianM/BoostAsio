#include <boost/asio.hpp>
#include <iostream>

// g++ -ansi -std=c++11 ServerCloseAndShutdown.cpp -o ServerCloseAndShutdown -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread -lboost_chrono

/*
The server application is intended to allocate an acceptor socket and passively wait for a connection request. When the connection request arrives, it should accept it and read the data from the socket connected to the client until the client application shuts down the socket on its side. Having received the request message, the server application should send the response message notifying its boundary by shutting down the socket.
*/


using namespace boost;


void processRequest(asio::ip::tcp::socket& sock) {
  // We use extensible buffer because we don't
  // know the size of the request message.
  asio::streambuf request_buf;

  system::error_code ec;

  // Receiving the request.
  asio::read(sock, request_buf, ec);

//   std::string str(request_buf.data().begin(), request_buf.data().end());

//   std::cout << "Request: " << str << std::endl;

  // Shutting down the socket in the client application is seen in the server application
  // as a protocol service message that arrives to the server,
  // notifying the fact that the peer application has shut down the socket.
  // Boost.Asio delivers this message to the application code
  // by means of an error code returned by the asio::read() function.
  // The Boost.Asio library defines this code as asio::error::eof.
  if (ec != asio::error::eof)
    throw system::system_error(ec);

  // Request received. Sending response.
  // Allocating and filling the buffer with
  // binary data.
  const char response_buf[] = { 0x48, 0x69, 0x21 };

  // Sending the request data.
  asio::write(sock, asio::buffer(response_buf));

  // Shutting down the socket to let the
  // client know that we've sent the whole
  // response.
  sock.shutdown(asio::socket_base::shutdown_send);
}

int main()
{
  unsigned short port_num = 3333;
  
  try {
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),
      port_num);

    asio::io_service ios;

    asio::ip::tcp::acceptor acceptor(ios, ep);

    asio::ip::tcp::socket sock(ios);

    acceptor.accept(sock);

    processRequest(sock);
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
