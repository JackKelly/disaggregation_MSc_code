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

BOOST_AUTO_TEST_CASE( GNUplotInstantiateTemplateTest )
{
    GNUplot gnuplot;

    GNUplotVars gnuPlotVars;
    gnuPlotVars.inFilename = "TEST_hist_with_power_states";
    gnuPlotVars.outFilename = "TEST_hist_with_power_states";
    gnuPlotVars.title = "This is a test";
    gnuPlotVars.xlabel = "power (Watts)";
    gnuPlotVars.ylabel = "frequency";
    gnuPlotVars.gnuPlotData.push_back( GNUplotData( "path\\/to\\/data\\/file1", "data title1" ) );
    gnuPlotVars.gnuPlotData.push_back( GNUplotData( "pathtodatafile2", "data title2" ) );

    gnuplot.instantiateTemplate( gnuPlotVars );
}
