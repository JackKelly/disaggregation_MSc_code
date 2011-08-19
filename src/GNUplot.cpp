/*
 * GNUplot.cpp
 *
 *  Created on: 18 Jul 2011
 *      Author: jack
 */

#include "GNUplot.h"
#include "Utils.h"
#include "Common.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <list>

using namespace std;


/**
 * @brief Plot a graph using GNUplot.
 *
 * First checks to see if
 * <tt>config/'plotVars.outFilename'.template.gnu</tt> exists.  If it does, it uses that as the template
 * (so individual devices can have their own specific templates). If it doesn't exist,
 * the code uses <tt>config/'plotVars.inFilename'.template.gnu</tt> as the template.
 *
 * @see GNUplot::PlotVars for details of what variables
 * are expected to be instantiated in plotVars.
 *
 */
void GNUplot::plot(
        PlotVars& plotVars /**< Variables for insertion into the template.
                                @c 'inFilename' will be changed if config/'inFilename'.template.gnu exists.
                                Strings will be sanitised. */
    )
{
    /* Check if config/'plotVars.outFilename'.template.gnu exists.
     * If so, use it as the template.  */
    if ( Utils::fileExists( string("config/" + plotVars.outFilename + ".template.gnu") ) ) {
        plotVars.inFilename = plotVars.outFilename;
    }

    sanitise( &plotVars ); // remove troublesome characters

    instantiateTemplate( plotVars ); // replace tokens in template with values from plotVars

    const string gnuPlotScriptFilename = DATA_OUTPUT_PATH + plotVars.outFilename + ".gnu";
    const string plotCommand = "gnuplot " + gnuPlotScriptFilename;

    cout << "Plotting gnuplot script " << gnuPlotScriptFilename << " to produce output "
         << DATA_OUTPUT_PATH << plotVars.outFilename << "." << GNUPLOT_OUTPUT_FILE_EXTENSION
         << endl;
    system( plotCommand.c_str() );
}

/**
 * @brief Replace all characters which might confuse 'sed'.
 *
 * In particular, this function replaces:
 *   '/'  ->   '\\/'
 */
void GNUplot::sanitise(
        PlotVars * pv_p /**< Input and output */
    )
{
    sanitise( &(pv_p->inFilename) );
    sanitise( &(pv_p->outFilename) );
    sanitise( &(pv_p->title) );
    sanitise( &(pv_p->xlabel) );
    sanitise( &(pv_p->ylabel) );
    sanitise( &(pv_p->data) );
}

void GNUplot::sanitise(
        list<Data> * data_p  /**< Input and output */
        )
{
    for (list<Data>::iterator item=data_p->begin(); item!=data_p->end(); item++) {
        sanitise( &(item->dataFile) );
        sanitise( &(item->title)    );
    }
}

void GNUplot::sanitise(
        string * str_p  /**< Input and output */
        )
{
    boost::algorithm::replace_all( *str_p, "/", "\\/" );
}

/**
 * @brief Replaces all the tokens in the template file 'config/plotVars.inFilename' with
 * variables from 'plotVars' and outputs the instantiated template as 'plotVars.outFilename'
 * in the directory defined by DATA_OUTPUT_PATH.
 *
 * Tokens replaced in template =
 *    TITLE
 *    XLABEL
 *    YLABEL
 *    SETTERMINAL
 *    SETOUTPUT
 *    PLOTARGS
 *
 *    for each datafile:
 *    <tokenbase>FILE
 *    <tokenbase>KEY
 *
 * The find-and-replace functionality is currently implemented using a 'system()' call
 * to 'sed'.  This is a bit of a hack but it was quick to implement, works perfectly
 * well and isn't a bottleneck.  In a perfect world, it'd be nice to implement a
 * find-and-replace template instantiation system using something like
 * <a href="http://www.boost.org/doc/libs/1_47_0/libs/regex/doc/html/boost_regex/ref/regex_replace.html">
 * Boost.Regex::regex_replace()</a> or Boost's String Algorithms.
 *
 */
void GNUplot::instantiateTemplate(
        const PlotVars& plotVars /**< Variables for insertion into the template. */
    )
{
    string sanitisedOutputPath = DATA_OUTPUT_PATH;
    sanitise( &sanitisedOutputPath );

    string sedCommand =
            "sed -e '"
             "s/TITLE/" + plotVars.title + "/g"
            ";s/XLABEL/" + plotVars.xlabel + "/g"
            ";s/YLABEL/" + plotVars.ylabel + "/g"
            ";s/SETTERMINAL/" + GNUPLOT_SET_TERMINAL + "/g"
            ";s/SETOUTPUT/set output \"" + sanitisedOutputPath
                     + plotVars.outFilename + "." + GNUPLOT_OUTPUT_FILE_EXTENSION + "\"/g"
            ";s/PLOTARGS/" + plotVars.plotArgs + "/g";

    /* Loop through each gnuPlotData list item...
     *     (would be nice to use C++0x's 'foreach' syntax, but that's only been
     *      supported in GCC since v4.6 but the DoC machines only run GCC v4.3) */
    size_t count = 0;
    string digit;
    for ( list<Data>::const_iterator data=plotVars.data.begin();
            data != plotVars.data.end();
            data++, count++ ) {

        string sanitisedDataFilename;
        if (data->useDefaults) {
            sanitisedDataFilename = sanitisedOutputPath + data->dataFile + ".dat";
        } else {
            sanitisedDataFilename = data->dataFile ;
        }

        digit = '1' + count; // convert size_t "count" to a single-digit string
        sedCommand +=
                ";s/" + data->tokenBase + "FILE/" + sanitisedDataFilename + "/g"
                ";s/" + data->tokenBase + "KEY/" + data->title + "/g";
    }

    sedCommand +=
            "' config/" + plotVars.inFilename + ".template.gnu" // input to sed
            " > " + DATA_OUTPUT_PATH + plotVars.outFilename + ".gnu";  // output from sed

    cout << "Instantiating GNUplot template \"config/" + plotVars.inFilename + ".template.gnu\""
            " and outputting instantiated template to " << DATA_OUTPUT_PATH << plotVars.outFilename << ".gnu" << endl;
    system( sedCommand.c_str() ) ;
}
