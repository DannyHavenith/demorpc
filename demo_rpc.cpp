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
#include "binary_function_wrapper.hpp"
#include "binary_function_marshaller.hpp"

//#include "connection.hpp"



using FunctionMap=std::map<std::string, std::shared_ptr<FunctionInterface>>;

int add( int left, int right)
{
    return left + right;
}

std::string addstrings( const std::string &left, const std::string &right)
{
    return left + right;
}

struct Information
{
    std::string first;
    std::string second;

    template<typename Archive>
    void serialize( Archive & ar, const unsigned int)
    {
        ar & first;
        ar & second;
    }
};

std::string addAll( const Information &inf)
{
    return inf.first + inf.second;
}

int main()
{

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

    return 0;
}
