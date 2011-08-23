/*
 * PowerStateSequence.cpp
 *
 *  Created on: 26 Jul 2011
 *      Author: jack
 */

#include "PowerStateSequence.h"
#include "Utils.h"
#include <string>
#include <iostream>
#include <string>

using namespace std;


void PowerStateSequence::setDeviceName(
        const std::string& _deviceName  /**< e.g. "Washer" or "Washer sigID1" */
        )
{
    deviceName = _deviceName;
}

void PowerStateSequence::plotGraph() const
{
    assert( ! deviceName.empty() );

    // Dump data to a .dat file
    const std::string baseFilename =
            getBaseFilename();

    cout << "Plotting graph for " << baseFilename << endl;

    dumpToFile( baseFilename );

    // Set plot variables
    GNUplot::PlotVars pv;
    pv.inFilename  = "powerStateSequence";
    pv.outFilename = baseFilename;
    pv.title       = baseFilename;
    pv.xlabel      = "time (Seconds)";
    pv.ylabel      = "power (Watts)";
    pv.plotArgs    = "";

    pv.data.push_back( GNUplot::PlotData( baseFilename, deviceName + " power state sequence", "POWERSTATESEQUENCE" ) );

    pv.data.push_back( GNUplot::PlotData( deviceName + "-afterCropping", deviceName + " raw signature (unsmoothed)", "SIG" ) );

    // Plot
    GNUplot::plot( pv );
}

const string PowerStateSequence::getBaseFilename() const
{
    assert( ! deviceName.empty() );
    return deviceName + "-powerStateSequence";
}

/**
 * @brief Dump the contents of powerStateSequence to a data file ready for
 * gnuplot to plot using the @c 'boxxyerrorbars' style
 * (see p42 of the gnuplot 4.4 documentation PDF).
 */
void PowerStateSequence::dumpToFile(
        const string baseFilename /**< Base filename.  Excluding path and suffix. */
        ) const
{
    // open datafile
    string filename =
            DATA_OUTPUT_PATH + baseFilename + ".dat";

    cout << "Dumping power state sequence to " << filename << endl;

    fstream dataFile;
    Utils::openFile( dataFile, filename.c_str(), fstream::out );

    // Output header information for data file
    dataFile << "# automatically produced by PowerStateSequence::dumpToFile function" << endl
             << "# " << Utils::todaysDateAndTime() << endl
             << "# x\ty\txlow\txhigh\tylow\tyhigh" << endl;

    // loop through the 'poewStateSequence' list
    for (PowerStateSequence::const_iterator powerState=this->begin();
            powerState!=this->end();
            powerState++ ) {

        /* The columns required by gnoplot's Xyerrorbars style are:
         * x  y  xlow  xhigh  ylow  yhigh
        */

        dataFile << (powerState->startTime + powerState->endTime)/2 << "\t"  // x
                 << (powerState->powerState->min + powerState->powerState->max)/2 << "\t"  // y
                 << powerState->startTime << "\t"         // xlow  (== start time)
                 << powerState->endTime   << "\t"         // xhigh (== end time)
                 << powerState->powerState->min << "\t"   // ylow  (== min value)
                 << powerState->powerState->max <<  endl; // yhigh (== max value)

    }

    dataFile.close();
}

