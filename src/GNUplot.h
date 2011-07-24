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

/**
 * Structure for storing details about data elements to be plotted.
 */
struct GNUplotData {
    std::string dataFile, /**< Data filename (including suffix but without path). Directory = DATA_OUTPUT_PATH config option. */
                title;    /**< Title of this data element (for displaying in the graph's key). */
    GNUplotData( const std::string& _dataFile, const std::string& _title )
    : dataFile(_dataFile), title(_title) {}
};

/**
 * Structure for storing variables destined for a GNUplot script.
 */
struct GNUplotVars {
    std::string inFilename,  /**< template filename (without suffix or path). Directory hard-coded to be '/config'. */
                outFilename, /**< output filename (without suffix or path). Directory = DATA_OUTPUT_PATH config option. */
                title,       /**< Graph title. */
                xlabel,
                ylabel;
    std::list<GNUplotData> gnuPlotData; /**< List of data elements to be plotted. */
};

class GNUplot {
public:
    GNUplot() throw( std::string) ;
    virtual ~GNUplot();

    void operator() (const std::string& command);

    const bool good();

    void instantiateTemplate(
            const GNUplotVars& gnuPlotVars /**< Variables for insertion into the template. */
    );

protected:
    FILE * gnuplotpipe;
    bool isGood;
};

#endif /* GNUPLOT_H_ */
