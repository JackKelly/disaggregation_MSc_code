/*
 * Histogram.cpp
 *
 *  Created on: 9 Sep 2011
 *      Author: jack
 */

#include "Histogram.h"

    /**
     * @brief Create a (relative) histogram from a sample array.
     */
    Histogram::Histogram(
            const Array<Sample_t>& source,
            const size_t xaxis /**< Number of elements on the xaxis of the histogram */
            )
    : Array<Histogram_t>(), sizeOfSource( source.getSize() )
    {
        upstreamSmoothing = source.getSmoothing();
        smoothing = 0;
        deviceName = source.getDeviceName();

        setSize( xaxis ); // 1 Watt resolution

        setAllEntriesTo(0);

        const Histogram_t quantum = (Histogram_t)1/sizeOfSource ;

        for (size_t i=0; i<sizeOfSource; i++) {
            data[ Utils::roundToNearestInt( source[i] ) ] += quantum;
        }
    }

    const std::string Histogram::getBaseFilename() const
    {
        std::string baseFilename = deviceName + "-hist" +
                (smoothing ? ("-HistSmoothing" + Utils::size_t_to_s(smoothing)) : "") +
                (upstreamSmoothing ? ("-UpstreamSmoothing" + Utils::size_t_to_s(upstreamSmoothing)) : "");
        return baseFilename;
    }

    void Histogram::drawGraph()
    {
        // Dump data to a .dat file
        const std::string baseFilename =
                getBaseFilename();

        dumpToFile( baseFilename );

        // Set plot variables
        GNUplot::PlotVars pv;
        pv.inFilename  = "histogram";
        pv.outFilename = baseFilename;
        pv.title       = baseFilename;
        pv.xlabel      = "power (Watts)";
        pv.ylabel      = "frequency";
        pv.plotArgs    = "";
        pv.data.push_back( GNUplot::PlotData( baseFilename, baseFilename ) );

        // Plot
        GNUplot::plot( pv );
    }

    const size_t Histogram::getSizeOfSource() const
    {
        return sizeOfSource;
    }
