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
                ylabel,
                plotArgs;    /**< Plot arguments (e.g. range). Best not to use this but to use the template.gnu file instead. */
    std::list<Data> data;    /**< List of data elements to be plotted. */
};

void plot(
        PlotVars& gnuPlotVars
);

void sanitise(
        PlotVars * pv_p
);

void sanitise(
        std::string * str_p
);

void sanitise(
        std::list<Data> * data_p
);

void instantiateTemplate(
        const PlotVars& gnuPlotVars
);

} /* namespace GNUplot */

#endif /* GNUPLOT_H_ */
