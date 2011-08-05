#define BOOST_TEST_MODULE AggregateData AggregateDataTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/AggregateData.h"
#include <boost/test/unit_test.hpp>
#include <iostream>


BOOST_AUTO_TEST_CASE( findTime )
{
    AggregateData aggData;
    aggData.loadCurrentCostData( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv" );

    BOOST_CHECK_EQUAL( aggData.findTime(1310252400), 0 );
    BOOST_CHECK_EQUAL( aggData.findTime(1310255190), 427);
    BOOST_CHECK_EQUAL( aggData.findTime(1310338795), 12988);
}
