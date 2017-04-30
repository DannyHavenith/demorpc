//
//  Copyright (C) 2017 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BINARY_FUNCTION_MARSHALLER_HPP_
#define BINARY_FUNCTION_MARSHALLER_HPP_

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "function_interface.hpp"
#include "tuple_serialization.hpp"



/**
 * Object type that acts as a functor and that:
 *     - serializes all parameters into a blob
 *     - calls a FunctionInterface instance with that parameter blob
 *     - translates the return blob to the given return type
 */
template<typename FunctionType>
class BinaryFunctionMarshaller
{
};

template<typename ReturnType, typename... Parameters>
class BinaryFunctionMarshaller< ReturnType (Parameters...)>
{
public:
    BinaryFunctionMarshaller( std::shared_ptr<FunctionInterface> function)
            :m_function{function}
    {
    }

    ReturnType operator()( Parameters... pars)
    {
        using namespace boost::iostreams;
        using namespace boost::archive;

        auto parameterTuple = std::make_tuple( pars...);
        Blob parameterBlob;
        stream<back_insert_device<Blob>> parameterStream{parameterBlob};
        binary_oarchive parameterArchive{ parameterStream};
        parameterArchive << parameterTuple;
        parameterStream.flush();

        Blob resultBlob = m_function->Call( parameterBlob);

        ReturnType result;
        stream<array_source> resultStream{ &resultBlob.front(), resultBlob.size()};
        binary_iarchive resultArchive{ resultStream};
        resultArchive >> result;

        return result;
    }
private:
    std::shared_ptr<FunctionInterface> m_function;
};

template<typename ReturnType, typename... Parameters>
BinaryFunctionMarshaller< ReturnType (Parameters...)> Marshal(
        ReturnType (*)( Parameters...),
        const std::shared_ptr<FunctionInterface> wrappedFunction
    )
{
    return BinaryFunctionMarshaller< ReturnType (Parameters...)>{wrappedFunction};
}



#endif /* BINARY_FUNCTION_MARSHALLER_HPP_ */
