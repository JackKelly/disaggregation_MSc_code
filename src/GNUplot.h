/*
 * GNUplot.h
 *
 *  Created on: 18 Jul 2011
 *      Author: jack
 */

#ifndef GNUPLOT_H_
#define GNUPLOT_H_

#include <string>
#include <list>

namespace GNUplot {

/**
 * Structure for storing details about data elements to be plotted.
 */
struct Data {
    std::string dataFile, /**< Data filename (including suffix but without path). Directory = DATA_OUTPUT_PATH config option. */
                title;    /**< Title of this data element (for displaying in the graph's key). */
    Data( const std::string& _dataFile, const std::string& _title )
    : dataFile(_dataFile), title(_title) {}
};

/**
 * Structure for storing variables destined for a GNUplot script.
 */
struct PlotVars {
    std::string inFilename,  /**< template filename (without suffix or path). Directory hard-coded to be '/config'. */
                outFilename, /**< output filename (without suffix or path). Directory = DATA_OUTPUT_PATH config option. */
                title,       /**< Graph title. */
                xlabel,
                ylabel;
    std::list<Data> data; /**< List of data elements to be plotted. */
};

void plot(
        PlotVars& gnuPlotVars /**< Variables for insertion into the template. */
);

void instantiateTemplate(
        const PlotVars& gnuPlotVars /**< Variables for insertion into the template. */
);

}

#endif /* GNUPLOT_H_ */
