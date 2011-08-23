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
#include "Common.h" // for DATA_OUTPUT_PATH

namespace GNUplot {

/**
 * @brief Structure for storing details about data elements to be plotted.
 */
struct PlotData {
    std::string dataFile, /**< @brief Data filename.  If this does not include path or suffix
                                      the set @c useDefaults=true, else set useDefaults=false. */
                title,    /**< @brief Title of this data element (for displaying in the graph's key). */
                tokenBase;/**< @brief Base of token to look for in the template.
                                      @c FILE and @c KEY will be appended to this tokenBase. */
    bool        useDefaults; /**< @brief Should @c DATA_OUTPUT_PATH be added to the front of
                                  @c dataFile and @c ".dat" be added to the end of ~c dataFile?
                                  Defaults to true. */
    /**
     * @brief Constructor
     */
    PlotData(
            const std::string& _dataFile,
            const std::string& _title,
            const std::string& _tokenBase = "DATA",
            const bool _useDefaults = true
                    )
    : dataFile(_dataFile), title(_title), tokenBase(_tokenBase), useDefaults(_useDefaults) {}
};

/**
 * @brief Structure for storing variables destined for a GNUplot script.
 */
struct PlotVars {
    std::string inFilename,  /**< @brief template filename (without suffix or path).
                                         Directory hard-coded to be @c '/config'. */
                outFilename, /**< @brief output filename (without suffix or path).
                                         Directory = @c DATA_OUTPUT_PATH config option. */
                title,       /**< @brief Graph title. */
                xlabel,
                ylabel,
                plotArgs;    /**< @brief Plot arguments (e.g. range).
                                         Best not to use this but to put custom args
                                         into the @c template.gnu file instead. */
    std::list<PlotData> data;    /**< @brief List of data elements to be plotted.
                                         @see GNUplot::Data for details. */
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
        std::list<PlotData> * data_p
);

void instantiateTemplate(
        const PlotVars& gnuPlotVars
);

} /* namespace GNUplot */

#endif /* GNUPLOT_H_ */
