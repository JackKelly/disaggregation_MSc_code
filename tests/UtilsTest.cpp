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

BOOST_AUTO_TEST_CASE( size_t_to_sTest )
{
    BOOST_CHECK_EQUAL( size_t_to_s(1234), "1234" );
    BOOST_CHECK_EQUAL( size_t_to_s(0)   ,    "0" );
}
