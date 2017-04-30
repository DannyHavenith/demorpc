//
//  Copyright (C) 2017 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef FUNCTION_INTERFACE_HPP_
#define FUNCTION_INTERFACE_HPP_

#include <vector>

using Byte = char; // char is chosen to ease iostream definitions
using Blob = std::vector<Byte>;

class FunctionInterface
{
public:

    virtual Blob Call(const Blob &parameters) = 0;

    virtual ~FunctionInterface(){}
};

#endif /* FUNCTION_INTERFACE_HPP_ */
