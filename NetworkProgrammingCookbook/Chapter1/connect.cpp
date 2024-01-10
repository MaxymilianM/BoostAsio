#include <boost/asio.hpp>
#include <iostream>

/*
The following algorithm descries steps required to perform in the TCP client application to connect an active socket to the server application:

1. Obtain the target server application's IP address and a protocol port number.
2. Create an object of the asio::ip::tcp::endpoint class from the IP address and the protocol port number obtained in step 1.
3. Create and open an active socket.
4. Call the socket's connect() method specifying the endpoint object created in step 2 as an argument.
5. If the method succeeds, the socket is considered connected and can be used to send and receive data to and from the server.
*/

using namespace boost;

int main()
{
  // Step 1. Assume that the client application has already
  // obtained the IP address and protocol port number of the
  // target server.
  std::string raw_ip_address = "127.0.0.1";
  unsigned short port_num = 3333;

  try {
    // Step 2. Creating an endpoint designating 
    // a target server application.
    asio::ip::tcp::endpoint
      ep(asio::ip::address::from_string(raw_ip_address),
      port_num);

    asio::io_service ios;

    // Step 3. Creating and opening a socket.
    asio::ip::tcp::socket sock(ios, ep.protocol());

    // Note that we didn't bind the socket to any local endpoint before connecting it.
    // This doesn't mean that the socket stays unbound. Before performing the connection establishment procedure,
    // the socket's connect() method will bind the socket to the endpoint consisting of an IP address
    // and a protocol port number chosen by the operating system.

    // Step 4. Connecting a socket.
    // The connection is performed synchronously, which means that the method blocks the caller thread
    // until either the connection operation is established or an error occurs.
    sock.connect(ep);

    // At this point socket 'sock' is connected to 
    // the server application and can be used
    // to send data to or receive data from it.
  }
  // Overloads of asio::ip::address::from_string() and 
  // asio::ip::tcp::socket::connect() used here throw
  // exceptions in case of error condition.
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();
    
    return e.code().value();
  }

  return 0;
}