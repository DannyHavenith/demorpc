/*
 * demo_functions.hpp
 *
 */

#ifndef DEMO_FUNCTIONS_HPP_
#define DEMO_FUNCTIONS_HPP_


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


#endif /* DEMO_FUNCTIONS_HPP_ */
