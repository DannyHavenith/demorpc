/*
 * rpc_proxy.hpp
 *
 *  Created on: May 1, 2017
 *      Author: danny.havenith
 */

#ifndef RPC_PROXY_HPP_
#define RPC_PROXY_HPP_
#include "connection.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <vector>
#include <boost/serialization/vector.hpp>
#include "connection.hpp" // Must come before boost/serialization headers.
#include "rpc_message.hpp"


/**
 * Objects of this class open a connection to an RPC Service.
 *
 * Typically, member functions of this class are not called directly, but
 * by FunctionProxy objects instead.
 *
 * This class does not support multithreading, i.e. only one call of the
 * call() member function can be active at any time.
 *
 * It is safe to create more than one instance of this class and call
 * the call() member function of those instances concurrently.
 */
class RpcProxy
{
public:
	/// Constructor starts the asynchronous connect operation.
	RpcProxy(
			boost::asio::io_service& io_service,
			const std::string& host,
			const std::string& service)
	: connection_(io_service)
	{
		// Resolve the host name into an IP address.
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(host, service);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
				resolver.resolve(query);

		boost::asio::connect(connection_.socket(), endpoint_iterator);
	}

	RpcReply call( const RpcMessage &message)
	{
		connection_.write( message);
		return connection_.read<RpcReply>();
	}

private:
	/// The connection to the server.
	connection connection_;
};

/**
 * This class stores a function name and implements the FunctionInterface interface
 *
 * Whenever the Call member function is called, it will create an RpcMessage
 * that includes the function name and then delegate the call to an RpcProxy.
 */
class FunctionProxy : public FunctionInterface
{
public:
	FunctionProxy( const std::string &name, RpcProxy &rpc)
	: m_functionName{ name}, m_rpcProxy( rpc)
	{
	}

    Blob Call(const Blob &parameters) override
	{
    	return m_rpcProxy.call( std::tie(m_functionName, parameters));
	}

    virtual ~FunctionProxy(){};
private:
	const std::string 	m_functionName;
	RpcProxy 			&m_rpcProxy;
};

/**
 * Create a function proxy from an existing function prototype.
 *
 * The function proxy acts as a regular function (functor), but
 * will forward all function calls to a remote server through the
 * RpcProxy object.
 *
 * @see BinaryFunctionMarshaller
 *
 */
template<typename ReturnType, typename... Parameters>
BinaryFunctionMarshaller< ReturnType (Parameters...)> CreateProxyFunction(
        ReturnType (*)( Parameters...),
		RpcProxy &rpcProxy,
		const std::string &functionName
    )
{
	auto proxy = std::make_shared<FunctionProxy>( functionName, rpcProxy);
    return BinaryFunctionMarshaller< ReturnType (Parameters...)>{proxy};
}

/**
 * Create a function proxy from an explicitly specified function
 * prototype.
 *
 * The function proxy acts as a regular function (functor), but
 * will forward all function calls to a remote server through the
 * RpcProxy object.
 *
 * @see BinaryFunctionMarshaller
 *
 */
template< typename FunctionType>
BinaryFunctionMarshaller< FunctionType> CreateProxyFunction(
    RpcProxy &rpcProxy,
    const std::string &functionName
    )
{
    auto proxy = std::make_shared<FunctionProxy>( functionName, rpcProxy);
    return BinaryFunctionMarshaller< FunctionType>{proxy};
}

#endif /* RPC_PROXY_HPP_ */
