#include <boost/asio.hpp>
#include <iostream>

/*
The following algorithm and corresponding code sample describes how to prepare a buffer
that can be used with the Boost.Asio socket's method that performs an output operation
such as asio::ip::tcp::socket::send() or the asio::write() free function:

1. Allocate a buffer. Note that this step does not involve any functionality or data types from Boost.Asio.
2. Fill the buffer with the data that is to be used as the output.
3. Represent the buffer as an object that satisfies the ConstBufferSequence concept's requirements.
4. The buffer is ready to be used with Boost.Asio output methods and functions.
*/

using namespace boost;

int main()
{
    // Step 1 and 2 in single line.
    const std::string buf ("Hello"); // 'buf' is the raw buffer. 

    // Step 3. Creating buffer representation that satisfies 
    // ConstBufferSequence concept requirements.
    asio::const_buffers_1 output_buf = asio::buffer(buf); // buf should be const

    // Alternative Step 3. To use string object for TCP send()
    // we need to convert it to ConstBufferSequence concept.
    asio::const_buffer asio_buf(buf.c_str(), buf.length());
    std::vector<asio::const_buffer> buffers_sequence;
    buffers_sequence.push_back(asio_buf);

    // Step 4. 'output_buf' is the representation of the
    // buffer 'buf' that can be used in Boost.Asio output
    // operations.

    return 0;
}