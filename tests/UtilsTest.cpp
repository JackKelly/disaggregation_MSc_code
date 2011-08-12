#define BOOST_TEST_MODULE Utils UtilsTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/Utils.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace Utils;

BOOST_AUTO_TEST_CASE( roundToNearestIntTest )
{
    BOOST_CHECK_EQUAL( roundToNearestInt( 0  ),  0 );
    BOOST_CHECK_EQUAL( roundToNearestInt(-0.1),  0 );
    BOOST_CHECK_EQUAL( roundToNearestInt(-0.6), -1 );
    BOOST_CHECK_EQUAL( roundToNearestInt( 0.1),  0 );
    BOOST_CHECK_EQUAL( roundToNearestInt( 0.7),  1 );
}

BOOST_AUTO_TEST_CASE( roundToNearestSizeTTest )
{
    BOOST_CHECK_EQUAL( roundToNearestSizeT( 0  ),  0 );
    BOOST_CHECK_EQUAL( roundToNearestSizeT(-0.1),  0 );
    BOOST_CHECK_EQUAL( roundToNearestSizeT(-0.6),  0 );
    BOOST_CHECK_EQUAL( roundToNearestSizeT( 0.1),  0 );
    BOOST_CHECK_EQUAL( roundToNearestSizeT( 0.7),  1 );
    BOOST_CHECK_EQUAL( roundToNearestSizeT( 1.7),  2 );
    BOOST_CHECK_EQUAL( roundToNearestSizeT( 1.0),  1 );
}

BOOST_AUTO_TEST_CASE( size_t_to_sTest )
{
    BOOST_CHECK_EQUAL( size_t_to_s(1234), "1234" );
    BOOST_CHECK_EQUAL( size_t_to_s(0)   ,    "0" );
}

BOOST_AUTO_TEST_CASE( roughlyEqualTest )
{
    BOOST_CHECK( roughlyEqual( 1  , 1  , 0  ) );
    BOOST_CHECK( roughlyEqual( 1.1, 1  , 0.1) );
    BOOST_CHECK(!roughlyEqual(-1.1, 1  , 0.1) );
    BOOST_CHECK( roughlyEqual(-1.1,-1  , 0.1) );
    BOOST_CHECK(!roughlyEqual( 1.2, 1  , 0.1) );
}

BOOST_AUTO_TEST_CASE( largestSizeTTest )
{
    std::cout << "Running largestTest case..." << std::endl;
    BOOST_CHECK_EQUAL( largest((size_t)4,(size_t)5), 5);
    BOOST_CHECK_EQUAL( largest((size_t)10,(size_t)50), 50);
    BOOST_CHECK_EQUAL( largest((size_t)10,(size_t)100), 100);
}

BOOST_AUTO_TEST_CASE( largestDoubleTest )
{
    std::cout << "Running largestTest case..." << std::endl;
    BOOST_CHECK_EQUAL( highest(  4.0,   5.0),   5.0 );
    BOOST_CHECK_EQUAL( highest(-10.2,  50.1),  50.1 );
    BOOST_CHECK_EQUAL( highest(-10.9, 100.4), 100.4);
}

BOOST_AUTO_TEST_CASE( smallestSizeTTest )
{
    std::cout << "Running smallestTest case..." << std::endl;
    BOOST_CHECK_EQUAL( smallest((size_t) 4,(size_t)  5),   4);
    BOOST_CHECK_EQUAL( smallest((size_t)10,(size_t) 50),  10);
    BOOST_CHECK_EQUAL( smallest((size_t)10,(size_t)100),  10);
}

BOOST_AUTO_TEST_CASE( smallestDoubleTest )
{
    std::cout << "Running smallestTest case..." << std::endl;
    BOOST_CHECK_EQUAL( lowest(  4.0,   5.0),   4.0 );
    BOOST_CHECK_EQUAL( lowest(-10.2,  50.1), -10.2 );
    BOOST_CHECK_EQUAL( lowest(-10.9, 100.4), -10.9 );
}

BOOST_AUTO_TEST_CASE( betweenTest )
{
    std::cout << "Running betweenTest case..." << std::endl;
    BOOST_CHECK( between( 4, 5, 4.5) );
    BOOST_CHECK( !between( 4, 5, 5.5) );
    BOOST_CHECK( between( 4, 5, 5) );
    BOOST_CHECK( between( 4, 5, 4) );
    BOOST_CHECK( between( -4, 5, 4) );
    BOOST_CHECK( !between( -4, -5, 4) );
    BOOST_CHECK( between( -4, -5, -4) );
    BOOST_CHECK( between( -4, -5, -4.5) );
    BOOST_CHECK( between( -4, -5, -5) );
}

BOOST_AUTO_TEST_CASE( withinTest )
{
    std::cout << "Running withinTest case..." << std::endl;
    BOOST_CHECK( within( 4, 5, 1) );
    BOOST_CHECK( within( 4, 5, 1.1) );
    BOOST_CHECK( !within( 4, 5, 0.9) );
    BOOST_CHECK( !within( -4, 5, 1) );
    BOOST_CHECK( !within( 4, -5, 1) );
    BOOST_CHECK( within( -4, -5, 1) );
    BOOST_CHECK( !within( -4, -5, 0.9) );
}
