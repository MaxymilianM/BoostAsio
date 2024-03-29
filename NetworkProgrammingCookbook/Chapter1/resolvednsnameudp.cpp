#include <boost/asio.hpp>
#include <iostream>

/*
The following algorithm describes steps required to perform in a client application
in order to resolve a DNS name to obtain IP addresses (zero or more) of hosts (zero or more)
running the server application that the client application wants to communicate with:

1. Obtain the DNS name and the protocol port number designating the server application and represent them as strings.
2. Create an instance of the asio::io_service class or use the one that has been created earlier.
3. Create an object of the resolver::query class representing a DNS name resolution query.
4. Create an instance of DNS name resolver class suitable for the necessary protocol.
5. Call the resolver's resolve() method, passing a query object created in step 3 to it as an argument.
*/

using namespace boost;

int main()
{
  // Step 1. Assume that the client application has already
  // obtained the DNS name and protocol port number and 
  // represented them as strings.
  std::string host = "samplehost.book";
  std::string port_num = "3333";

  // Step 2.
  asio::io_service ios;

  // Step 3. Creating a query.
  asio::ip::udp::resolver::query resolver_query(host,
      port_num, asio::ip::udp::resolver::query::numeric_service);

  // Step 4. Creating a resolver.
  asio::ip::udp::resolver resolver(ios);

  // Used to store information about error that happens
  // during the resolution process.
  boost::system::error_code ec;

  // Step 5.
  asio::ip::udp::resolver::iterator it =
      resolver.resolve(resolver_query, ec);

  // Handling errors if any.
  if (ec.value() != 0) {
    // Failed to resolve the DNS name. Breaking execution.
    std::cout << "Failed to resolve a DNS name."
              << "Error code = " << ec.value() 
              << ". Message = " << ec.message();

    return ec.value();
  }

  asio::ip::udp::resolver::iterator it_end;

  /*
  Usually, when a DNS name of the host running the server application is resolved to more than one IP address
  and correspondingly to more than one endpoint, the client application doesn't know which one of the multiple endpoints to prefer.
  The common approach in this case is to try to communicate with each endpoint one by one, until the desired response is received.
  */

  for (; it != it_end; ++it) {
    // Here we can access the endpoint like this.
    asio::ip::udp::endpoint ep = it->endpoint();
  }

  return 0;
}
