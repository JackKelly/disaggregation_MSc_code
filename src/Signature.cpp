/*
 * Signature.cpp
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#include "Signature.h"
#include "Common.h"
#include "Utils.h"
#include "Statistic.h"
#include <glog/logging.h>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <list>

using namespace std;

/**
 * Constructor for opening a CSV file
 *
 */
Signature::Signature(
        const char* filename,   /**< Filename, including path and suffix. */
        const size_t _samplePeriod, /**< Sample period. In seconds. @todo sample period should be read from data file. */
        const size_t _sigID,    /**< Each Device can have multiple signatures. A Device's first sig gets a sigID of 0, the next gets a sigID of 1 etc. Default=0. */
        const size_t cropFront, /**< Number of elements to crop from the front. Default=0. If 0 then will automatically crop 0s from the front such that there's only a single leading zero left. */
        const size_t cropBack   /**< Number of elements to crop from the back. Default=0. If 0 then will automatically crop 0s from the back such that there's only a single trailing zero left. */
        )
: samplePeriod(_samplePeriod), sigID(_sigID)
{
    fstream dataFile;
    Utils::openFile( dataFile, filename, fstream::in );

    Array<Sample_t> data;
    data.loadData( dataFile );
    dataFile.close();

    if ( cropFront == 0 ) {
        copyCrop( data, data.getNumLeadingZeros(), data.getNumTrailingZeros() );
    } else {
        copyCrop( data, cropFront, cropBack );
    }
}

Signature::~Signature()
{}


/**
 * @todo this needs to be changed.  Firstly, Sig will be a child of Array,  The function should
 * automatically determine the history as much as possible by following the 'upstream' pointers.
 */
void Signature::drawHistWithStateBars(
        const Array<Histogram_t>& hist, /**< Histogram array. */
        const size_t raLength,          /**< Length of the rolling average used to smooth hist gradient. */
        const string& deviceName        /**< The human-readable name of the device. */
    )
{

    // Dump data to a .dat file

    const std::string histogramFilename = DATA_OUTPUT_PATH + deviceName + "-histogram-" + ".dat";
    hist.dumpToFile( histogramFilename );

    // Set plot variables
    GNUplot::PlotVars pv;
    pv.inFilename  = "histWithStateBars";
    pv.outFilename = deviceName + "-histWithStateBars-gradSmoothing-" + Utils::size_t_to_s(raLength); /**< @todo smoothing of original data */
    pv.title       = deviceName + " histogram with state bars. Rolling average length=" + Utils::size_t_to_s(raLength);  /**< @todo smoothing of original data */
    pv.xlabel      = "power (Watts)";
    pv.ylabel      = "histogram frequency";
    pv.data.push_back( GNUplot::Data( histogramFilename, "Histogram. Smoothing = " ) );

    // Plot
/*    GNUplot::plot( pv );

              "\"data/processed/hist.dat\" with l lw 1 title \"raw histogram\", "
              "\"data/processed/RA_hist.dat\" with l lw 1 title \""
            + Utils::size_t_to_s(raLength) + "-step rolling average of histogram gradient\", "
              "\"data/processed/x_err_bars.dat\" with xerrorbars title \"automatically determined state boundaries (min, mean, max)\" " );
*/
}

/**
 * Runs through 'rawReading' to find power states.  'rawReading' must be
 * populated before this function is called.
 */
const PowerStates_t Signature::getPowerStates( const size_t rollingAvLength ) const
{
    PowerStates_t powerStates;

    LOG(INFO) << "Finding power states...rollingAvLength = " << rollingAvLength;
    assert ( size ); // make sure Signature is populated

//    drawGraph( "rawReading", "time (seconds)", "power (Watts)", "[] []" );

    // Create a histogram

    Array<Sample_t> RA;
    rollingAv( &RA, rollingAvLength );
    RA.drawGraph( "rollingAv", "time (seconds)", "power (Watts)", "[] []" );
    Histogram hist ( RA );

//    hist.drawGraph( description, "power (Watts)", "frequency", "[0:2500] [0:100]" );

    // find the boundaries of the different power states in the histogram
    std::list<size_t> boundaries;
    hist.findPeaks( &boundaries );

    // calculate proper stats for power states...
    assert( (boundaries.size() % 2)==0 ); // check there's an even number of entries

    // Go through each pair of boundaries, working out stats for each
    size_t front, back;
    fstream dataFile;
    Utils::openFile( dataFile, "data/processed/x_err_bars.dat", fstream::out );
    for (std::list<size_t>::const_iterator it=boundaries.begin(); it!=boundaries.end(); it++) {

        front = *it;
        back  = *(++it);

        powerStates.push_back( PowerState_t( hist, front, back ) );

        std::cout << powerStates.back() << std::endl;

        powerStates.back().xErrorBarOutputLine( dataFile );
    }
    dataFile.close();
//    drawHistWithStateBars( hist, rollingAvLength );

    return powerStates;
}

/**
 * Fill in gaps in the powerStates so that each powerState in the list is nose-to-tail
 */
void Signature::fillGapsInPowerStates( const Array<Histogram_t>& hist )
{
    assert( ! powerStates.empty() );

    PowerStates_t::iterator prevPowerState,
                            powerState;

    prevPowerState = powerState = powerStates.begin();
    powerState++;

    for ( ; powerState!=powerStates.end(); powerState++) {

        if (powerState->min != prevPowerState->max &&
            powerState->min != (prevPowerState->max + 1) ) {
            // 'prevPowerState' and this 'powerState' are not nose-to-tail so insert a new power state

            LOG(INFO) << "Inserting new power state between " << prevPowerState->max << " and " << powerState->min;

            // Insert new powerState before current powerState
            powerStates.insert( powerState,
                                PowerState_t( hist, (prevPowerState->max + 1), powerState->min ) );
        } // end if

        prevPowerState = powerState;
    }

    // deal with case where final powerState still doesn't cover upper range
    if (prevPowerState->max != MAX_WATTAGE) {
        powerStates.push_back( PowerState_t( hist, (prevPowerState->max + 1), MAX_WATTAGE ) );
    }

    // debugging code
    for (powerState = powerStates.begin(); powerState!=powerStates.end(); powerState++) {
        cout << "Power state = " << *powerState << std::endl;
    }

}

/**
 * Convert from the Signature data to a different sample period.
 * If the original sample period is 1 second and newPeriod = 6, then the returned
 * array will have 1/6th the number of entries and each entry in the downsampled
 * array will be an average of 6 samples from the original array.
 *
 * @deprecated Usually use rollingAv()
 *
 * @todo this should be in Array shouldn't it?
 *
 * @return output
 */
void Signature::downSample( Array<Sample_t> * output, const size_t newPeriod ) const
{
    size_t inner, inputIndex, outputIndex, outerLimit;
    Sample_t accumulator;

    LOG(INFO) << "Resampling...";

    const size_t newSize = (int)ceil(size / ( (double)newPeriod / (double)samplePeriod ));
    output->setSize( newSize );


    const size_t mod = newPeriod % samplePeriod;
    if (mod==0) {
        // newPeriod % samplePeriod == 0; so then just take an average of each step
        const size_t stepSize = newPeriod/samplePeriod;
        const size_t delta = (newSize*newPeriod) - (size*samplePeriod);

        if (delta==0) {
            outerLimit = newSize;
        } else {
            outerLimit = newSize-1;
        }

        LOG(INFO) << "old size=" << size << ", old period=" << samplePeriod
                  << ", new size=" << newSize << ", newPeriod=" << newPeriod
                  << ", stepSize=" << stepSize << ", mod=" << mod << ", delta=" << delta << ",outerLimit=" << outerLimit;


        // Do the vast majority of the resampling
        for (outputIndex=0; outputIndex<outerLimit; outputIndex++) {
            accumulator=0;
            for (inner=0; inner<stepSize; inner++) {
                inputIndex = inner+(outputIndex*stepSize);
                accumulator += data[ inputIndex ];
            }
            (*output)[ outputIndex ] = accumulator/stepSize;
        }

        // Do the remainder (if there is any)
        if (delta != 0) {
            accumulator=0;
            size_t i;
            for (i=inputIndex; i<size; i++) {
                accumulator += data[i];
                LOG(INFO) << "i=" << i << ", data[i]=" << data[i];
            }
            (*output)[ newSize-1 ] = accumulator/(delta-1);
            LOG(INFO) << "Filled remainder.  output[" << newSize-1 << "]=" << (*output)[newSize-1];
        }

    } else {
        // newPeriod%samplePeriod!=0 so interpolate
        LOG(ERROR) << "resample() doesn't yet deal with fractional sample rate conversions";
    }
}

const size_t Signature::getSamplePeriod() const
{
    return samplePeriod;
}
