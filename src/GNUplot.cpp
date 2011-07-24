/*
 * GNUplot.cpp
 *
 *  Created on: 18 Jul 2011
 *      Author: jack
 */

#include "GNUplot.h"
#include <string>
#include <stdio.h>
#include <iostream>

using namespace std;

GNUplot::GNUplot() throw( string )
{
    gnuplotpipe = popen( "gnuplot" , "w" );
    if ( gnuplotpipe ) {
        isGood = true;
    } else {
        throw ("Gnuplot not found!");
        isGood = false;
    }

}

GNUplot::~GNUplot()
{
    operator()( "exit" );
    pclose( gnuplotpipe );
}

void GNUplot::operator()(const string& command)
{
    fprintf( gnuplotpipe, "%s\n", command.c_str() );
    fflush( gnuplotpipe );
}

const bool GNUplot::good()
{
    return isGood;
}

/**
 * @brief Opens 'gnuPlotVars.inFilename', replaces all the tokens in the template file with
 * variables from 'gnuPlotVars' and outputs the instantiated template as 'gnuPlotVars.outFilename'.
 *
 * The find-and-replace functionality is currently implemented using a 'system()' call
 * to 'sed'.  This is a bit of a hack but it was quick to implement, works perfectly
 * well and isn't a bottleneck.  In a perfect world, it'd be nice to implement a
 * find-and-replace template instantiation system using something like
 * <a href="http://www.boost.org/doc/libs/1_47_0/libs/regex/doc/html/boost_regex/ref/regex_replace.html">
 * Boost.Regex::regex_replace()</a>.
 */
void GNUplot::instantiateTemplate(
        const GNUplotVars& gnuPlotVars /**< Variables for insertion into the template. */
    )
{
    string sedCommand =
            "sed -e '"
             "s/TITLE/" + gnuPlotVars.title + "/g"
            ";s/XLABEL/" + gnuPlotVars.xlabel + "/g"
            ";s/YLABEL/" + gnuPlotVars.ylabel + "/g";

    /* Loop through each gnuPlotData list item...
     *     (would be nice to use C++0x's 'foreach' syntax, but that's only been
     *      supported in GCC since v4.6, any the DoC machines only run GCC v4.3) */
    size_t count = 0;
    string digit;
    for ( list<GNUplotData>::const_iterator data=gnuPlotVars.gnuPlotData.begin();
            data != gnuPlotVars.gnuPlotData.end();
            data++, count++ ) {

        digit = '1' + count;
        sedCommand +=
                ";s/DATAFILE" + digit + "/" + data->dataFile + "/g"
                ";s/DATAKEY"+ digit + "/" + data->title + "/g";

    }

    sedCommand +=
            "' config/" + gnuPlotVars.inFilename + ".template.gnu"
            " > data/output/" + gnuPlotVars.outFilename + ".gnu"; /**< @todo add path from config option DATA_OUTPUT_PATH */

    cout << sedCommand << endl;
    system( sedCommand.c_str() ) ;
}
