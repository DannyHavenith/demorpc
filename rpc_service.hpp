/*
 * rpc_service.hpp
 *
 *  Created on: May 1, 2017
 *      Author: danny.havenith
 */

#ifndef RPC_SERVICE_HPP_
#define RPC_SERVICE_HPP_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include "connection.hpp"
#include <boost/serialization/vector.hpp>
#include "binary_function_wrapper.hpp"

/**
 * An RpcService object has a map of string->FunctionInterface pointers. It
 * will listen for connections and then for each connection start a sequence of
 * reading RpcMessages, calling the appropriate function and writing RpcReplies
 * with the function results.
 */
class RpcService
{
public:
    /// Constructor opens the acceptor and starts waiting for the first incoming
    /// connection.
    RpcService(boost::asio::io_service& io_service, unsigned short port)
    :m_acceptor(io_service,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {

            // Start an accept operation for a new connection.
            connection_ptr new_conn(new connection(m_acceptor.get_io_service()));
            m_acceptor.async_accept(new_conn->socket(),
                boost::bind(&RpcService::handle_accept, this,
                    boost::asio::placeholders::error, new_conn));
    }

    /**
     * Register a FunctionInterface instance by name.
     */
    void register_function( const std::string &name, const std::shared_ptr<FunctionInterface> &function)
    {
        m_functions[name] = function;
    }

    /**
     * Register a free function by name.
     *
     * This will create a wrapper on top of the function that implements the FunctionInterface
     * interface.
     */
    template< typename FunctionType>
    void register_function(
        const std::string &name,
        FunctionType function)
    {
        register_function( name, Wrap( function));
    }

    /// Handle completion of a accept operation.
    void handle_accept(const boost::system::error_code& e, connection_ptr conn)
    {
        using namespace boost::placeholders;

        if (!e)
        {
            conn->async_read<RpcMessage>(
                [this, conn](const boost::system::error_code& e, const RpcMessage &message = {})
                {
                handle_read( e, conn, message);
                }
            );
        }

        // Start an accept operation for a new connection.
        connection_ptr new_conn(new connection(m_acceptor.get_io_service()));
        m_acceptor.async_accept(new_conn->socket(),
            boost::bind(&RpcService::handle_accept, this,
                boost::asio::placeholders::error, new_conn));
    }

    /**
     * Handle the completion of an RpcMessage read operation.
     *
     * This will try to find the corresponding function for the message and call it.
     */
    void handle_read(const boost::system::error_code& e, connection_ptr conn, const RpcMessage &message = {})
    {
        using std::get;
        if (!e)
        {
            // we received an RpcMessage, call the corresponding function.
            // and send the result back to the receiver.
            auto result = m_functions[get<0>(message)]->Call(get<1>(message));
            conn->async_write(
                result,
                boost::bind(&RpcService::handle_write, this,
                    boost::asio::placeholders::error, conn));

            // also start a read for the next message.
            conn->async_read<RpcMessage>(
                [this, conn](const boost::system::error_code& e, const RpcMessage &message = {})
                {
                    handle_read( e, conn, message);
                }
            );
        }
        else
        {
            // do nothing, read failed is a normal condition when the
            // other side closes the connection.
        }
    }

    /**
     * Handle a finished write of the function results.
     *
     * This does nothing except for some error handling, the read of the next message was already
     * scheduled.
     */
    void handle_write(const boost::system::error_code& e, connection_ptr conn)
    {
        if (e)
        {
            std::cerr << "write failed: " << e.message() << '\n';
        }
    }

private:
    /// The acceptor object used to accept incoming socket connections.
    typedef std::map< std::string, std::shared_ptr<FunctionInterface>> FunctionMap;
    FunctionMap                       m_functions;
    boost::asio::ip::tcp::acceptor    m_acceptor;

};


#endif /* RPC_SERVICE_HPP_ */
