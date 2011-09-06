#define BOOST_TEST_MODULE GNUplot GNUplotTest
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/GNUplot.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <list>


BOOST_AUTO_TEST_CASE( GNUplotInstantiateTemplateTest )
{
    GNUplot::PlotVars plotVars;
    plotVars.inFilename = "TEST_plot";
    plotVars.outFilename = "TEST_specific_plot";
    plotVars.title = "This is a test";
    plotVars.xlabel = "time (Seconds)";
    plotVars.ylabel = "power (Watts)";
    plotVars.data.push_back( GNUplot::PlotData( "data/input/watts_up/washer.csv", "Washer 1", "DATA", false ) );
    plotVars.data.push_back( GNUplot::PlotData( "data/input/watts_up/washer2.csv", "Washer 2", "DATA", false ) );

    GNUplot::plot( plotVars );
}
