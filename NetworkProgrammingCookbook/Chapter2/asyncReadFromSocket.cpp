#include <boost/asio.hpp>
#include <iostream>

// g++ -ansi -std=c++11 asyncReadFromSocket.cpp -o asyncReadFromSocket -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread -lboost_chrono

/*
The following algorithm describes the steps required to implement an application,
which reads data from a socket asynchronously.
Note that this algorithm provides a possible way to implement such an application.
Boost.Asio is quite flexible and allows us to organize and structure the application
by reading data from a socket asynchronously in different ways:

1. Define a data structure that contains a pointer to a socket object, a buffer, a variable that defines the size of the buffer, and a variable used as a counter of bytes read.
2. Define a callback function that will be called when an asynchronous reading operation is completed.
3. In a client application, allocate and open an active TCP socket, and then, connect it to a remote application. In a server application, obtain a connected active TCP socket by accepting a connection request.
4. Allocate a buffer big enough for the expected message to fit in.
5. Initiate an asynchronous reading operation by calling the socket's async_read_some() method, specifying a function defined in step 2 as a callback.
6. Call the run() method on an object of the asio::io_service class.
7. In a callback, increase the counter of bytes read. If the number of bytes read is less than the total amount of bytes to be read (that is, the size of an expected message), initiate a new asynchronous reading operation to read the next portion of data.
*/

using namespace boost;

// Step 1. Keeps objects we need in a callback to
// identify whether all data has been read
// from the socket and to initiate next async
// reading operation if needed.
struct Session {
  std::shared_ptr<asio::ip::tcp::socket> sock;
  std::unique_ptr<char[]> buf;
  std::size_t total_bytes_read;
  unsigned int buf_size;
};

// Step 2. Function used as a callback for 
// asynchronous reading operation.
// Checks if all data has been read
// from the socket and initiates
// new reading operation if needed.
void callback(const boost::system::error_code& ec,
    std::size_t bytes_transferred,
    std::shared_ptr<Session> s)
{
  if (ec.value() != 0) {
    std::cout << "Error occured! Error code = "
      << ec.value()
      << ". Message: " << ec.message();

    return;
  }

  s->total_bytes_read += bytes_transferred;

  if (s->total_bytes_read == s->buf_size) {
    return;
  }

  // If there expected more data we call async_read_some that will call the callback after.
  s->sock->async_read_some(asio::buffer(s->buf.get() + s->total_bytes_read,
                                        s->buf_size - s->total_bytes_read),
                                        std::bind(callback,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2, 
                                                  s));
}

void readFromSocket(std::shared_ptr<asio::ip::tcp::socket> sock) {  
  std::shared_ptr<Session> s(new Session);

  // Step 4. Allocating the buffer.
  const unsigned int MESSAGE_SIZE = 7;

  s->buf.reset(new char[MESSAGE_SIZE]);
  s->total_bytes_read = 0;
  s->sock = sock;
  s->buf_size = MESSAGE_SIZE;

  // Step 5. Initiating asynchronous reading operation.
  s->sock->async_read_some(asio::buffer(s->buf.get(), s->buf_size),
                            std::bind(callback,
                                      std::placeholders::_1,
                                      std::placeholders::_2,
                                      s));
  // Because we want to pass an additional argument to our callback function,
  // a pointer to the corresponding Session object, which acts as a context for the operation,
  // we use the std::bind() function to construct a function object to which we attach a pointer to the Session object as the third argument.
}

struct Session2 {
  std::shared_ptr<asio::ip::tcp::socket> sock;
  std::unique_ptr<char[]> buf;
  unsigned int buf_size;
}; 

void callback2(const boost::system::error_code& ec,
  std::size_t bytes_transferred,
  std::shared_ptr<Session2> s)
{
  if (ec.value() != 0) {
    std::cout << "Error occured! Error code = "
      << ec.value()
      << ". Message: " << ec.message();

    return;
  }

  // Here we know that the reading has completed
  // successfully and the buffer is full with
  // data read from the socket.
}


void readFromSocket2(std::shared_ptr<asio::ip::tcp::socket> sock) {  
  std::shared_ptr<Session2> s(new Session2);

  const unsigned int MESSAGE_SIZE = 7;

  s->buf.reset(new char[MESSAGE_SIZE]);
  s->sock = sock;
  s->buf_size = MESSAGE_SIZE;

  asio::async_read(*(s->sock),
                    asio::buffer(s->buf.get(), s->buf_size),
                    std::bind(callback2,
                                std::placeholders::_1,
                                std::placeholders::_2,
                                s));
}

int main()
{
  std::string raw_ip_address = "127.0.0.1";
  unsigned short port_num = 3333;

  try {
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address),
                               port_num);

    asio::io_service ios;

    // Step 3. Allocating, opening and connecting a socket.
    std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(ios, ep.protocol()));

    sock->connect(ep);

    readFromSocket(sock);

    readFromSocket2(sock);

    // Step 6. The asio::io_service::run() method blocks as long as there is at least one pending asynchronous operation.
    // When the last callback of the last pending operation is completed, this method returns.
    ios.run();
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
}
