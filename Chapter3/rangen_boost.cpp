// #include <boost/random/mersenne_twister.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <iostream>

// Build with mersenne_twister:
// g++ -ansi rangen_boost.cpp -I /home/max/sources/boost_1_58_0 -o rangen_boost

// Build with random_device:
// 1. Add path for linker to find lboost_random and lboost_system:
// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
// 2.1. Compile with sources path:
// g++ -ansi rangen_boost.cpp -I /home/max/sources/boost_1_58_0 -L /usr/local/lib/ -lboost_random -lboost_system -o rangen2_boost
// 2.2. Compile with default path if Boost was installed:
// g++ -ansi rangen_boost.cpp -I /usr/local/include -L /usr/local/lib/ -lboost_random -lboost_system -o rangen2_boost

int main(void) {
  int guessNumber;
  std::cout << "Select number among 0 to 10: ";
  std::cin >> guessNumber;
  if(guessNumber < 0 || guessNumber > 10) {
    return 1;
  }
  //  boost::random::mt19937 rng;
  boost::random::random_device rng;
  boost::random::uniform_int_distribution<> ten(0,10);
  int randomNumber = ten(rng);
  if(guessNumber == randomNumber) {
    std::cout << "Congratulation, " << guessNumber << " is your lucky number.\n";
  }
  else {
    std::cout << "Sorry, I'm thinking about number " << randomNumber << "\n"; 
  }
  return 0;
}
