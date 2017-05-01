//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Connection type for RPC proxy and _service. This is an adaptation of
// the serialization exampl of boost::asio. Original copyright:
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERIALIZATION_CONNECTION_HPP
#define SERIALIZATION_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include <iomanip>
#include <string>
#include <vector>


/// The connection class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * @li An 8-byte header containing the length of the serialized data in
 * hexadecimal.
 * @li The serialized data.
 */
class connection
{
public:
    typedef std::vector<char> Blob;

    /// Constructor.
    connection(boost::asio::io_service& io_service)
    : socket_(io_service)
    {
    }

    /// Get the underlying socket. Used for making a connection or for accepting
    /// an incoming connection.
    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    /// Asynchronously write a data structure to the socket.
    template <typename T, typename Handler>
    void async_write(const T& t, Handler handler)
    {
        using namespace boost::iostreams;
        using namespace boost::archive;

        // Serialize the data first so we know how large it is.
        outbound_data_.clear();
        stream<back_insert_device<Blob>> dataStream{ outbound_data_};
        binary_oarchive archive{ dataStream};

        archive << t;

        dataStream.flush();
        // Format the header.
        std::ostringstream header_stream;
        header_stream << std::setw(header_length)
                      << std::hex << outbound_data_.size();
        if (!header_stream || header_stream.str().size() != header_length)
        {
            // Something went wrong, inform the caller.
            boost::system::error_code error(boost::asio::error::invalid_argument);
            socket_.get_io_service().post(boost::bind(handler, error));
            return;
        }
        outbound_header_ = header_stream.str();

        // Write the serialized data to the socket. We use "gather-write" to send
        // both the header and the data in a single write operation.
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outbound_header_));
        buffers.push_back(boost::asio::buffer(outbound_data_));
        boost::asio::async_write(socket_, buffers, handler);
    }

    template <typename T>
    void write( const T& t)
    {
        using namespace boost::iostreams;
         using namespace boost::archive;

         // Serialize the data first so we know how large it is.
         outbound_data_.clear();
         stream<back_insert_device<Blob>> dataStream{ outbound_data_};
         binary_oarchive archive{ dataStream};

         archive << t;
         dataStream.flush();

         // Format the header.
         std::ostringstream header_stream;
         header_stream << std::setw(header_length)
                       << std::hex << outbound_data_.size();

         if (!header_stream || header_stream.str().size() != header_length)
         {
             throw boost::system::error_code{ boost::asio::error::invalid_argument};
         }
         outbound_header_ = header_stream.str();

         // Write the serialized data to the socket. We use "gather-write" to send
         // both the header and the data in a single write operation.
         std::vector<boost::asio::const_buffer> buffers;
         buffers.push_back(boost::asio::buffer(outbound_header_));
         buffers.push_back(boost::asio::buffer(outbound_data_));
         boost::asio::write(socket_, buffers);
    }

    /// Asynchronously read a data structure from the socket.
    template <typename T, typename Handler>
    void async_read(Handler handler)
    {
        // Issue a read operation to read exactly the number of bytes in a header.
        void (connection::*f)(
                const boost::system::error_code&,
                boost::tuple<Handler>)
                = &connection::handle_read_header<T, Handler>;
        boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
                boost::bind(f,
                        this, boost::asio::placeholders::error,
                        boost::make_tuple(handler)));
    }

    // read an object of type T synchronously.
    template< typename T>
    T read()
    {
    	T result;
        using namespace boost::iostreams;
        using namespace boost::archive;
    	boost::asio::read( socket_, boost::asio::buffer( inbound_header_));
        std::istringstream is(std::string(inbound_header_, header_length));
        std::size_t inbound_data_size = 0;
        if (!(is >> std::hex >> inbound_data_size))
        {
            // Header doesn't seem to be valid. Inform the caller.
            throw boost::system::error_code{boost::asio::error::invalid_argument};
        }

        inbound_data_.resize(inbound_data_size);
        boost::asio::read( socket_, boost::asio::buffer( inbound_data_));

        stream<basic_array_source<char>> dataStream{ &inbound_data_[0], inbound_data_.size()};
        binary_iarchive archive{ dataStream};

        archive >> result;

        return result;
    }

    /// Handle a completed read of a message header. The handler is passed using
    /// a tuple since boost::bind seems to have trouble binding a function object
    /// created using boost::bind as a parameter.
    template <typename T, typename Handler>
    void handle_read_header(const boost::system::error_code& e,
        boost::tuple<Handler> handler)
    {
        if (e)
        {
            boost::get<0>(handler)(e);
        }
        else
        {
            // Determine the length of the serialized data.
            std::istringstream is(std::string(inbound_header_, header_length));
            std::size_t inbound_data_size = 0;
            if (!(is >> std::hex >> inbound_data_size))
            {
                // Header doesn't seem to be valid. Inform the caller.
                boost::system::error_code error(boost::asio::error::invalid_argument);
                boost::get<0>(handler)(error);
                return;
            }

            // Start an asynchronous call to receive the data.
            inbound_data_.resize(inbound_data_size);
            void (connection::*f)(
                    const boost::system::error_code&,
                    boost::tuple<Handler>)
                    = &connection::handle_read_data<T, Handler>;
            boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
                    boost::bind(f, this,
                            boost::asio::placeholders::error, handler));
        }
    }

    /// Handle a completed read of message data.
    template <typename T, typename Handler>
    void handle_read_data(const boost::system::error_code& e,
            boost::tuple<Handler> handler)
    {
        if (e)
        {
            boost::get<0>(handler)(e);
        }
        else
        {
        	T t;
            // Extract the data structure from the data just received.
            try
            {
                using namespace boost::iostreams;
                using namespace boost::archive;

                // Serialize the data first so we know how large it is.
                stream<basic_array_source<char>> dataStream{ &inbound_data_[0], inbound_data_.size()};
                binary_iarchive archive{ dataStream};

                archive >> t;
            }
            catch (std::exception& e)
            {
                // Unable to decode data.
                boost::system::error_code error(boost::asio::error::invalid_argument);
                boost::get<0>(handler)(error);
                return;
            }

            // Inform caller that data has been received ok.
            boost::get<0>(handler)(e, t);
        }
    }

private:
    /// The underlying socket.
    boost::asio::ip::tcp::socket socket_;

    /// The size of a fixed length header.
    enum { header_length = 8 };

    /// Holds an outbound header.
    std::string outbound_header_;

    /// Holds the outbound data.
    Blob outbound_data_;

    /// Holds an inbound header.
    char inbound_header_[header_length];

    /// Holds the inbound data.
    Blob inbound_data_;
};

typedef boost::shared_ptr<connection> connection_ptr;


#endif // SERIALIZATION_CONNECTION_HPP
