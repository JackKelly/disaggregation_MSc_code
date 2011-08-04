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
    BOOST_CHECK_CLOSE( stat.getStdev(), 2.7167908239, 0.00000001 );
    BOOST_CHECK_EQUAL( stat.getMax(),  10 );
    BOOST_CHECK_EQUAL( stat.getMin(),   1 );
}

BOOST_AUTO_TEST_CASE( similarTest )
{
    // car mileage data from http://www.itl.nist.gov/div898/handbook/eda/section3/eda3531.htm
    // the means are different

    int pop[] = {18,15,18,16,17,15,14,14,14,15,15,14,15,14,22,18,21,21,10,10,11,9,28,25,19,16,17,19,18,14,14,14,14,12,13,13,18,22,19,18,23,26,25,20,21,13,14,15,14,17,11,13,12,13,15,13,13,14,22,28,13,14,13,14,15,12,13,13,14,13,12,13,18,16,18,18,23,11,12,13,12,18,21,19,21,15,16,15,11,20,21,19,15,26,25,16,16,18,16,13,14,14,14,28,19,18,15,15,16,15,16,14,17,16,15,18,21,20,13,23,20,23,18,19,25,26,18,16,16,15,22,22,24,23,29,25,20,18,19,18,27,13,17,13,13,13,30,26,18,17,16,15,18,21,19,19,16,16,16,16,25,26,31,34,36,20,19,20,19,21,20,25,21,19,21,21,19,18,19,18,18,18,30,31,23,24,22,20,22,20,21,17,18,17,18,17,16,19,19,36,27,23,24,34,35,28,29,27,34,32,28,26,24,19,28,24,27,27,26,24,30,39,35,34,30,22,27,20,18,28,27,34,31,29,27,24,23,38,36,25,38,26,22,36,27,27,32,28,31};
    Array<int> src(sizeof(pop)/sizeof(int), pop);
    Statistic<int> stat( src );

    // create a second array and stat:

    int pop2[] = {24,27,27,25,31,35,24,19,28,23,27,20,22,18,20,31,32,31,32,24,26,29,24,24,33,33,32,28,19,32,34,26,30,22,22,33,39,36,28,27,21,24,30,34,32,38,37,30,31,37,32,47,41,45,34,33,24,32,39,35,32,37,38,34,34,32,33,32,25,24,37,31,36,36,34,38,32,38,32};
    Array<int> src2(sizeof(pop2)/sizeof(int), pop2);
    Statistic<int> stat2( src2 );

    BOOST_CHECK( ! stat.similar(stat2) );
}

BOOST_AUTO_TEST_CASE( similarTest2 )
{
    // car mileage data from http://www.itl.nist.gov/div898/handbook/eda/section3/eda3531.htm

    int pop[] = {5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5};
    Array<int> src(sizeof(pop)/sizeof(int), pop);
    Statistic<int> stat( src );

    // create a second array and stat:

    int pop2[] = {4,5,6,5,6,5,4,5,6,5,4,5,5,4,6,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5,4,5,6,5,4,5,5,4,6,6,5,4,5};
    Array<int> src2(sizeof(pop2)/sizeof(int), pop2);
    Statistic<int> stat2( src2 );

    BOOST_CHECK( stat2.similar(stat) );
}

BOOST_AUTO_TEST_CASE( singleStatsTest )
{
    std::cout << "Running singleStatsTest..." << std::endl;

    Statistic<size_t> stat( (size_t)10 );

    BOOST_CHECK_EQUAL( stat.getMean(), 10 );
    BOOST_CHECK_EQUAL( stat.getStdev(), 0 );
    BOOST_CHECK_EQUAL( stat.getMin(), 10 );
    BOOST_CHECK_EQUAL( stat.getMax(), 10 );
    BOOST_CHECK_EQUAL( stat.getNumDataPoints(), 1 );

    // now add in another data point
    stat.update(15);
    BOOST_CHECK_EQUAL( stat.getMean(), 12.5 );
    BOOST_CHECK_CLOSE( stat.getStdev(), 3.5355339059, 0.0000001 );
    BOOST_CHECK_EQUAL( stat.getMin(), 10 );
    BOOST_CHECK_EQUAL( stat.getMax(), 15 );
    BOOST_CHECK_EQUAL( stat.getNumDataPoints(), 2 );

    // now add in another data point
    stat.update(5);
    BOOST_CHECK_CLOSE( stat.getMean(), 10.0, 0.00000001 );
    BOOST_CHECK_CLOSE( stat.getStdev(), 5.0, 0.00000001 );
    BOOST_CHECK_EQUAL( stat.getMin(),   5 );
    BOOST_CHECK_EQUAL( stat.getMax(),  15 );
    BOOST_CHECK_EQUAL( stat.getNumDataPoints(), 3 );

}
