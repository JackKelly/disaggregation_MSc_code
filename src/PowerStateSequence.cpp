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

using namespace std;

/*
PowerStateSequence::PowerStateSequence() {

}

PowerStateSequence::~PowerStateSequence() {

}
*/

/**
 * Dump the contents of powerStateSequence to a data file ready for
 * gnuplot to plot using the 'boxxyerrorbars' style (see p42 of the gnuplot 4.4 documentation PDF).
 *
 */
void PowerStateSequence::dumpToFile(
        const string& details /**< Details to be appended onto filename.  e.g. device name and signature ID. */
        ) const
{
    // open datafile
    string filename = DATA_OUTPUT_PATH + "-powerStateSequence.dat";

    cout << "Dumping power state sequence to " << filename;

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

