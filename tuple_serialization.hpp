//
//  Copyright (C) 2017 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef TUPLE_SERIALIZATION_HPP_
#define TUPLE_SERIALIZATION_HPP_
#include <tuple>
#include <boost/serialization/level.hpp>
#include "index_sequence.hpp"

using TwoInts = std::tuple<int,int>;

BOOST_CLASS_IMPLEMENTATION(TwoInts, boost::serialization::object_serializable);

namespace boost {
    namespace serialization {

    template <typename... Elements>
    struct implementation_level_impl<std::tuple<Elements...> >
    :public mpl::int_< boost::serialization::object_serializable >
    {
    };

    }
}

template<typename Archive, typename TupleType, size_t... Indexes>
void SerializeTuple( Archive &ar, TupleType &tuple, IndexSequence<Indexes...> )
{
    (void)(int[]){ 0, ( (ar & std::get<Indexes>(tuple)),0)...
    };
}

namespace boost {
namespace serialization {
template< typename Archive, typename... Elements>
void serialize( Archive &ar, std::tuple<Elements...> &tuple, const unsigned int)
{
    SerializeTuple( ar, tuple, MakeIndexSequence_t< sizeof...(Elements)>{});
}
}
}




#endif /* TUPLE_SERIALIZATION_HPP_ */
