#define BOOST_TEST_MODULE GNUplot GNUplotTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/GNUplot.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <list>

BOOST_AUTO_TEST_CASE( GNUplotConstructorTest )
{
    GNUplot gnuplot;
    BOOST_CHECK( gnuplot.good() );
}
