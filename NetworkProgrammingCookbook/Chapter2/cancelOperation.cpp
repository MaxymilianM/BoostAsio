#include <boost/predef.h> // Tools to identify the OS.

#ifdef BOOST_OS_WINDOWS
#define _WIN32_WINNT 0x0501

#if _WIN32_WINNT <= 0x0502 // Windows Server 2003 or earlier.
#define BOOST_ASIO_DISABLE_IOCP
#define BOOST_ASIO_ENABLE_CANCELIO  
#endif
#endif

#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using namespace boost;

// g++ -ansi -std=c++11 cancelOperation.cpp -o cancelOperation -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread -lboost_chrono

/*
The following algorithm provides the steps required to initiate and cancel asynchronous operations with Boost.Asio:

1. If the application is intended to run on Windows XP or Windows Server 2003, define flags that enable asynchronous operation canceling on these versions of Windows.
2. Allocate and open a TCP or UDP socket. It may be an active or passive (acceptor) socket in the client or server application.
3. Define a callback function or functor for an asynchronous operation. If needed, in this callback, implement a branch of code that handles the situation when the operation has been canceled.
4. Initiate one or more asynchronous operations and specify a function or an object defined in step 4 as a callback.
5. Spawn an additional thread and use it to run the Boost.Asio event loop.
6. Call the cancel() method on the socket object to cancel all the outstanding asynchronous operations associated with this socket.
*/

int main()
{
  std::string raw_ip_address = "127.0.0.1";
  unsigned short port_num = 4444;

  try {
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address),
                               port_num);

    asio::io_service ios;

    std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(ios, ep.protocol()));

    sock->async_connect(ep,
      [sock](const boost::system::error_code& ec)
    {
      // If asynchronous operation has been
      // cancelled or an error occured during
      // execution, ec contains corresponding
      // error code.
      if (ec.value() != 0) {
        if (ec == asio::error::operation_aborted) {
          std::cout << "Operation cancelled!";
        }
        else {
          std::cout << "Error occured!"
            << " Error code = " << ec.value()
            << ". Message: " << ec.message() << std::endl;
        }

        return;
      }
      // At this point the socket is connected and
      // can be used for communication with 
      // remote application.
    });

    // Starting a thread, which will be used
    // to call the callback when asynchronous 
    // operation completes.
    std::thread worker_thread([&ios](){
      try {
        ios.run();
      }
      catch (system::system_error &e) {
        std::cout << "Error occured!"
        << " Error code = " << e.code()
        << ". Message: " << e.what();
      }
    });

    // Emulating delay.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Cancelling the initiated operation.
    sock->cancel();

    // When the callback function returns, the worker thread exits the event loop
    // because there are no more outstanding asynchronous operations to be executed.
    // As a result, the thread exits its entry point function.
    // This leads to the main thread running to its completion as well. Eventually, the application exits.

    // Waiting for the worker thread to complete.
    worker_thread.join();
  }
  catch (system::system_error &e) {
    std::cout << "Error occured! Error code = " << e.code()
      << ". Message: " << e.what();

    return e.code().value();
  }

  return 0;
}