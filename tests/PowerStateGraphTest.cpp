#define BOOST_TEST_MODULE PowerStateGraph PowerStateGraphTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/PowerStateGraph.h"
#include "../src/Statistic.h"
#include "../src/Array.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE( constructorTest )
{
    std::cout << "constructorTest..." << std::endl;
    PowerStateGraph psg;
}

BOOST_AUTO_TEST_CASE( updateTest )
{
    std::cout << "updateTest..." << std::endl;

    PowerStateGraph psg;

    //    Signature sig( "data/input/watts_up/kettle.csv", 1, "kettle" );
    //    Signature sig( "data/input/watts_up/toaster.csv", 1, "toaster" );
    //    Signature sig( "data/input/watts_up/tumble.csv", 1, "tumble", 1, 1, 6600 );
        Signature sig( "data/input/watts_up/washer.csv", 1, "washer", 1, 1, 2530 );
        Signature sig2( "data/input/watts_up/washer2.csv", 1, "washer2", 1,1, 2000 );

        psg.update( sig );
        psg.update( sig2 );

        std::cout << psg << std::endl;
        psg.writeGraphViz( std::cout );
}
