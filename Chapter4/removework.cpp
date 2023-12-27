#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

// g++ -ansi -std=c++11 removework.cpp -o removework -I /usr/local/include -L /usr/local/lib/ -lboost_system -lboost_thread -lpthread

// 
// We can also unblock the program by removing the work object from the io_service object,
// but we have to use a pointer to the work object in order to remove the work object itself.
// We are going to use the shared_ptr pointer, a smart pointer provided by the Boost libraries.
// Let's use the modified code of blocked.cpp

// boost::shared_ptr<...>
// As we can see in the preceding code, we used the shared_ptr pointer to instantiate the work object.
// With this smart pointer provided by Boost, we no longer need to manually delete memory allocation
// in order to store the pointer since it guarantees that the object pointed to will be deleted when the last pointer is destroyed or reset.
// Do not forget to include shared_ptr.hpp inside the boost directory as the shared_ptr pointer is defined in the header file.

// reset()
// We also add the reset() function to reset the io_service object when it prepares for a subsequent run() function invocation (call).
// The reset() function has to be invoked before any invocation of the run() or poll() functions.
// It will also tell the shared_ptr pointer to automatically destroy the pointer we created.

// The preceding program explains that we have successfully removed the work object from the io_service object.
// We can use this functionality if we intend to finish all the pending work even though it hasn't actually been finished yet.

int main(void) {
  boost::asio::io_service io_svc;
  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(io_svc)
  );

  // Destroy the pointer we created
  worker.reset();

  io_svc.run();

  std::cout << "We will not see this line in console window :(" << std::endl;

  return 0;
}
