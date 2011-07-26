/*
 * GNUplot.cpp
 *
 *  Created on: 18 Jul 2011
 *      Author: jack
 */

#include "GNUplot.h"
#include "Utils.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <list>

using namespace std;

/**
 * @brief Plot a graph using GNUplot.  See GNUplot::PlotVars for details of what variables
 * are expected to be instantiated in plotVars.  First checks to see if
 * config/'plotVars.outFilename'.template.gnu exists.  If it does, it uses that as the template
 * (so individual devices can have their own specific templates). If it doesn't exist,
 * the code uses config/'plotVars.inFilename'.template.gnu as the template.
 *
 * @todo add path from config option DATA_OUTPUT_PATH.
 */
void GNUplot::plot(
        PlotVars& plotVars /*!< Variables for insertion into the template.
                                'inFilename' will be changed if config/'inFilename'.template.gnu exists.
                                Strings will be sanitised. */
    )
{

    /* Check if config/'plotVars.outFilename'.template.gnu exists.
     * If so, use it as the template.  */
    if ( Utils::fileExists( string("config/" + plotVars.outFilename + ".template.gnu") ) ) {
        plotVars.inFilename = plotVars.outFilename;
    }

    sanitise( plotVars ); // remove troublesome characters

    instantiateTemplate( plotVars );

    string plotCommand =
            "gnuplot data/output/" + plotVars.outFilename + ".gnu";

    system( plotCommand.c_str() );
}

/**
 * @brief Replace all characters which might confuse 'sed'.
 *
 * In particular, this function replaces:
 *   '/'  ->   '\\/'
 */
void GNUplot::sanitise(
        PlotVars& pv /**< Input and output */
    )
{
    sanitise( pv.inFilename  );
    sanitise( pv.outFilename );
    sanitise( pv.title );
    sanitise( pv.xlabel );
    sanitise( pv.ylabel );
    sanitise( pv.data );
}

void GNUplot::sanitise(
        list<Data>& data  /**< Input and output */
        )
{
    for (list<Data>::iterator item=data.begin(); item!=data.end(); item++) {
        sanitise( item->dataFile );
        sanitise( item->title    );
    }
}

void GNUplot::sanitise(
        string& str  /**< Input and output */
        )
{
    boost::algorithm::replace_all( str, "/", "\\/" );
}

/**
 * @brief Replaces all the tokens in the template file 'config/plotVars.inFilename' with
 * variables from 'plotVars' and outputs the instantiated template as 'plotVars.outFilename'
 * in the directory defined by DATA_OUTPUT_PATH.
 *
 * The find-and-replace functionality is currently implemented using a 'system()' call
 * to 'sed'.  This is a bit of a hack but it was quick to implement, works perfectly
 * well and isn't a bottleneck.  In a perfect world, it'd be nice to implement a
 * find-and-replace template instantiation system using something like
 * <a href="http://www.boost.org/doc/libs/1_47_0/libs/regex/doc/html/boost_regex/ref/regex_replace.html">
 * Boost.Regex::regex_replace()</a> or Boost's String Algorithms.
 *
 * @todo add path from config option DATA_OUTPUT_PATH.
 * @todo replace SETTERMINAL and SETOUTPUT
 */
void GNUplot::instantiateTemplate(
        const PlotVars& plotVars /**< Variables for insertion into the template. */
    )
{
    string sedCommand =
            "sed -e '"
             "s/TITLE/" + plotVars.title + "/g"
            ";s/XLABEL/" + plotVars.xlabel + "/g"
            ";s/YLABEL/" + plotVars.ylabel + "/g"
            ";s/SETTERMINAL/set terminal svg size 1200 800; set samples 1001/g"
            ";s/SETOUTPUT/set output \"" + plotVars.outFilename + ".svg\"/g"
            ";s/PLOTARGS/" + plotVars.plotArgs + "/g";

    /* Loop through each gnuPlotData list item...
     *     (would be nice to use C++0x's 'foreach' syntax, but that's only been
     *      supported in GCC since v4.6 but the DoC machines only run GCC v4.3) */
    size_t count = 0;
    string digit;
    for ( list<Data>::const_iterator data=plotVars.data.begin();
            data != plotVars.data.end();
            data++, count++ ) {

        digit = '1' + count; // convert size_t "count" to a single-digit string
        sedCommand +=
                ";s/DATAFILE" + digit + "/" + data->dataFile + "/g"
                ";s/DATAKEY"+ digit + "/" + data->title + "/g";
    }

    sedCommand +=
            "' config/" + plotVars.inFilename + ".template.gnu" // input to sed
            " > data/output/" + plotVars.outFilename + ".gnu";  // output from sed

    cout << sedCommand << endl;
    system( sedCommand.c_str() ) ;
}
