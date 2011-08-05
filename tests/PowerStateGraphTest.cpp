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
/*
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
*/
}

BOOST_AUTO_TEST_CASE( updateVerticesTest )
{
    std::cout << "updateVerticesTest..." << std::endl;

    PowerStateGraph psg;

//    Signature sig( "data/input/watts_up/kettle.csv", 1, "kettle" );
//    Signature sig( "data/input/watts_up/tumble.csv", 1, "tumble" );
    Signature sig( "data/input/watts_up/washer.csv", 1, "washer" );
    psg.updateVertices( sig );
//    psg.updateEdges( sig );


//    psg.updateVertices( washer );

//    Signature washer2( "data/input/watts_up/washer2.csv", 1, "washer2" );
//    psg.updateVertices( washer2 );
//    psg.updateEdges( washer2 );

    std::cout << psg << std::endl;


    psg.writeGraphViz( std::cout );

}
