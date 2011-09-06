#define BOOST_TEST_MODULE AggregateData AggregateDataTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/AggregateData.h"
#include "../src/Statistic.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <list>


BOOST_AUTO_TEST_CASE( findTime )
{
    AggregateData aggData;
    aggData.loadCurrentCostData( "data/input/current_cost/10July.csv" );

    BOOST_CHECK_EQUAL( aggData.findTime(1310252400), 0 );
    BOOST_CHECK_EQUAL( aggData.findTime(1310255190), 427);
    BOOST_CHECK_EQUAL( aggData.findTime(1310338795), 12988);
}

BOOST_AUTO_TEST_CASE( findSpike )
{
    AggregateData aggData;
    aggData.loadCurrentCostData( "data/input/current_cost/10July.csv" );

    std::list<AggregateData::FoundSpike> foundSpikes = aggData.findSpike(Statistic<Sample_t>(245));

/*    for (std::list<AggregateData::FoundSpike>::iterator spike = foundSpikes.begin();
            spike != foundSpikes.end();
            spike++) {
        std::cout << *spike << std::endl;
    }
*/

    BOOST_CHECK_EQUAL( foundSpikes.front().timestamp , 1310253570);

    std::cout << "done" << std::endl;
}
