#define BOOST_TEST_MODULE Statistic StatisticTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/Statistic.h"
#include "../src/Array.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE( constructorAndUpdateTest )
{

    const size_t SIZE = 10;
    int pop[SIZE] = {1,2,3,4,5,6,7,8,9,10};
    Array<int> src(SIZE, pop);

    Statistic<int> stat( src );

    BOOST_CHECK_EQUAL( src.getSize(), SIZE );
    BOOST_CHECK_EQUAL( src[9], 10 );
    BOOST_CHECK_EQUAL( stat.getNumDataPoints(), SIZE );
    BOOST_CHECK_EQUAL( stat.getMean(),  5.5 );
    BOOST_CHECK_CLOSE( stat.getStdev(), 3.027650354, 0.00000001 );
    BOOST_CHECK_EQUAL( stat.getMax(),  10 );
    BOOST_CHECK_EQUAL( stat.getMin(),   1 );

    // now try update

    const size_t SIZE2 = 5;
    int pop2[SIZE2] = {2,4,6,8,5};
    Array<int> src2(SIZE2, pop2);

    stat.update( src2 );

    BOOST_CHECK_EQUAL( stat.getNumDataPoints(), SIZE+SIZE2 );
    BOOST_CHECK_CLOSE( stat.getMean(),  5.3333333333, 0.00000001 );
    BOOST_CHECK_CLOSE( stat.getStdev(), 2.7167908239, 0.15 );
    BOOST_CHECK_EQUAL( stat.getMax(),  10 );
    BOOST_CHECK_EQUAL( stat.getMin(),   1 );


}

