//
//  Copyright (C) 2017 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BINARY_FUNCTION_WRAPPER_HPP_
#define BINARY_FUNCTION_WRAPPER_HPP_

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

#include "function_interface.hpp"
#include "index_sequence.hpp"


/**
 * Wrapper object that wraps any callable object and implements a
 * FunctionInterface interface.
 *
 * When called, this wrapper will de-serialize the argument Blob, then call
 * the wrapped function and then serialize the return value into a Blob.
 */
template< typename ReturnType, typename... Parameters>
class BinaryFunctionWrapper : public FunctionInterface
{
public:
    using Function = std::function< ReturnType( Parameters...)>;
    using ParameterTuple =
            std::tuple<
                typename std::remove_const<
                        typename std::remove_reference<Parameters>::type
                    >::type...
                >;

    BinaryFunctionWrapper( Function f)
    :m_function{f}
    {
    }

    Blob Call( const Blob &parameters) override
    {
        using namespace boost::iostreams;
        using namespace boost::archive;


        ParameterTuple pars;
        stream<array_source> parameterStream{ &parameters.front(), parameters.size()};
        binary_iarchive parameterArchive{ parameterStream};
        parameterArchive >> pars;

        Blob resultBlob;
        stream<back_insert_device<Blob>> resultStream{ resultBlob};
        binary_oarchive resultArchive{resultStream};
        auto res = Invoke( m_function, pars);

        resultArchive << res;

        return resultBlob;
    }

    virtual ~BinaryFunctionWrapper() {}

private:
    template< typename FunctionType, typename TupleType, size_t... Indexes>
    static ReturnType Invoke( FunctionType f, TupleType &tuple, IndexSequence<Indexes...>)
    {
        return f( std::get<Indexes>(tuple)...);
    }

    template< typename FunctionType, typename... Pars>
    static ReturnType Invoke( FunctionType f, std::tuple<Pars...> &parameters)
    {
        return Invoke( f, parameters, MakeIndexSequence_t<sizeof...(Pars)>{});
    }

    Function m_function;
};

//template< typename ReturnType, typename... Parameters>
//std::shared_ptr<FunctionInterface> Wrap( ReturnType (&function)( Parameters... pars))
//{
//    return std::make_shared<BinaryFunctionWrapper<ReturnType, Parameters...>>(function);
//}

template< typename ReturnType, typename... Parameters>
std::shared_ptr<FunctionInterface> Wrap( ReturnType (*function)( Parameters... pars))
{
    return std::make_shared<BinaryFunctionWrapper<ReturnType, Parameters...>>(function);
}

template< typename ObjectType, typename ReturnType, typename... Parameters>
std::shared_ptr<FunctionInterface> Wrap( std::shared_ptr<ObjectType> &object, ReturnType (ObjectType::*function)( Parameters... pars))
{
    return std::make_shared<BinaryFunctionWrapper<ReturnType, Parameters...>>(function);
}

#endif /* BINARY_FUNCTION_WRAPPER_HPP_ */
