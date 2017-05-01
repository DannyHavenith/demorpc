//
//  Copyright (C) 2017 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//



#include <iostream>
#include <string>
#include <map>

#include "function_interface.hpp"
#include "binary_function_marshaller.hpp"
#include "binary_function_wrapper.hpp"

#include "demo_functions.hpp"

#include "rpc_proxy.hpp"
#include "rpc_service.hpp"


// call functions on the remote server
void client( const std::string &host, const std::string &port)
{
	boost::asio::io_service io_service;
	RpcProxy proxy{ io_service, host, port};

	// create a function proxy by giving an explicit function prototype
    auto remoteAdd = CreateProxyFunction<int (int, int)>( proxy, "add");

    // create a function proxy by giving an example function prototype.
    // Note that this will never call the addAll function locally, it's
    // just used to provide a function prototype.
	auto remoteAddAll = CreateProxyFunction( addAll, proxy, "addAll");

	// call the functions on the remote server.
    std::cout << remoteAdd( 40,2 ) << '\n';
    std::cout << remoteAddAll( {"hello there, ", "world!"}) << '\n';
}

// start a service that implements a number of registered functions.
void server( unsigned short port)
{
    std::cout << "Running service on port " << port << '\n';

	boost::asio::io_service io_service;
    RpcService service{ io_service, port};

    // register two functions
    service.register_function( "addAll", addAll);
    service.register_function( "add", add);

    io_service.run(); // wait for incoming calls.
}

// just run some functions in-proc via the rpc mechanism.
void inproc()
{
    using FunctionMap=std::map<std::string, std::shared_ptr<FunctionInterface>>;

    FunctionMap functions;
    functions["add"] = Wrap( add);
    functions["addstrings"] = Wrap( addstrings);
    functions["addAll"] = Wrap( addAll);

    auto wrappedAdd = Marshal( add, functions["add"]);
    std::cout << wrappedAdd( 39, 3) << '\n';

    auto wrappedStrings = Marshal( addstrings, functions["addstrings"]);
    std::cout << wrappedStrings( "hello ", "world") << '\n';

    auto wrappedAddAll = Marshal( addAll, functions["addAll"]);
    std::cout << wrappedAddAll( {"hello ", "there"});
}

int main( int argc, const char *argv[])
{
	if (argc >= 2)
	{
	    if (argv[1] == std::string("server"))
	    {
	        server( 65432);
	    }
	    else
	    {
	        client("localhost", "65432");
	    }
	}
	else
	{
	    inproc();
	}


    return 0;
}
