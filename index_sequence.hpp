//
//  Copyright (C) 2017 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef INDEX_SEQUENCE_HPP_
#define INDEX_SEQUENCE_HPP_

template< size_t... Indexes>
struct IndexSequence {};

/// Quickie implementation of an index sequence (for demo on pre c++14 compiler)
template< size_t Count, size_t Start = 0, size_t... IndexesSoFar>
struct MakeIndexSequence
{
    typedef typename MakeIndexSequence<Count, Start + 1, IndexesSoFar..., Start>::type
            type;
};

template< size_t Count, size_t... IndexesSoFar>
struct MakeIndexSequence< Count, Count, IndexesSoFar...>
{
    typedef IndexSequence< IndexesSoFar...> type;
};

template< size_t Count>
using MakeIndexSequence_t = typename MakeIndexSequence< Count>::type;






#endif /* INDEX_SEQUENCE_HPP_ */
