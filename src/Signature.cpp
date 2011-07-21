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
Signature::Signature(const char* filename, const size_t _samplePeriod, const size_t cropFront, const size_t cropBack )
: samplePeriod(_samplePeriod)
{
    fstream dataFile;
    Utils::openFile( dataFile, filename, fstream::in );

    SigArray_t data;
    data.loadData( dataFile );
    dataFile.close();

    if ( cropFront == 0 ) {
        rawReading.copyCrop( data, findNumLeadingZeros( data ), findNumTrailingZeros( data ) );
    } else {
        rawReading.copyCrop( data, cropFront, cropBack );
    }
}

Signature::~Signature()
{}

const PowerStates_t& Signature::getPowerStates( const size_t rollingAvLength )
{
    if ( powerStates.empty() ) {
        findPowerStates( rollingAvLength );
    }

    return powerStates;
}

void Signature::drawGraphWithStateBars( const HistogramArray_t& hist, const size_t raLength )
{
    // dump data to temp file
    hist.dumpToFile( "data/processed/hist.dat" );

    const std::string name("histWithStateBars");

    GNUplot gnu_plot;

    gnu_plot( "set title \"" + name + "\"" );
    gnu_plot( "set terminal svg size 1200 800" );
    gnu_plot( "set samples 1001" ); // high quality
    gnu_plot( "set output \"data/diagrams/" + name + ".svg\"" );
    gnu_plot( "set xlabel \"power (Watts)\"" );
    gnu_plot( "set ylabel \"frequency\"" );
    gnu_plot( "plot [0:2500]  "
              "\"data/processed/hist.dat\" with l lw 1 title \"raw histogram\", "
              "\"data/processed/RA_hist.dat\" with l lw 1 title \""
            + Utils::size_t_to_s(raLength) + "-step rolling average of histogram gradient\", "
              "\"data/processed/x_err_bars.dat\" with xerrorbars title \"automatically determined state boundaries (min, mean, max)\" " );

    // output for LaTeX
    gnu_plot( "set format \"$%g$\"" );
    gnu_plot( "set terminal epslatex colour solid size 15cm, 10cm" );
    gnu_plot( "set output \"data/diagrams/" + name + ".tex\"" );
    gnu_plot( "plot [0:2400] "
              "\"data/processed/hist.dat\" with l lw 1 title \"raw histogram\", "
              "\"data/processed/RA_hist.dat\" with l lw 1 title \""
              + Utils::size_t_to_s(raLength) + "-step rolling average of histogram gradient\", ""-step rolling average of histogram gradient\", "
              "\"data/processed/x_err_bars.dat\" with xerrorbars title \"automatically determined state boundaries (min, mean, max)\" " );

}

/**
 * Runs through 'rawReading' to find power states.  'rawReading' must be
 * populated before this function is called.
 */
void Signature::findPowerStates( const size_t rollingAvLength )
{
    LOG(INFO) << "Finding power states...rollingAvLength = " << rollingAvLength;
    assert ( rawReading.size ); // make sure rawReading is populated

    rawReading.drawGraph( "rawReading", "time (seconds)", "power (Watts)", "[] []" );

    // Create a histogram
    HistogramArray_t hist;
    string description;

    if (rollingAvLength > 1) {
        RollingAv_t rollingAv;
        rawReading.rollingAv( &rollingAv, rollingAvLength );
        rollingAv.drawGraph( "rollingAv", "time (seconds)", "power (Watts)", "[] []" );
        rollingAv.histogram( &hist );
        description = "Hist_from_" + Utils::size_t_to_s(rollingAvLength) + "-step_rollingAv";
    } else {
        rawReading.histogram( &hist );
        description = "Hist_from_raw";
    }

    hist.drawGraph( description, "power (Watts)", "frequency", "[0:2500] [0:100]" );

//    RollingAv_t RAhist;
//    hist.rollingAv( &RAhist, 21 );
//    RAhist.drawGraph( "RA10_of_Hist_from_raw", "power (Watts)", "frequency", "[0:2500] [0:100]" );

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
    drawGraphWithStateBars( hist, rollingAvLength );

//    fillGapsInPowerStates( hist );
}

/**
 * Fill in gaps in the powerStates so that each powerState in the list is nose-to-tail
 */
void Signature::fillGapsInPowerStates( const HistogramArray_t& hist )
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

const SigArray_t& Signature::getRawReading() const
{
    return rawReading;
}

const size_t Signature::findNumLeadingZeros( const SigArray_t& data )
{
    size_t count = 0;
    while ( data[count]==0 && count<data.size ) {
        count++;
    }
    return count;
}

const size_t Signature::findNumTrailingZeros( const SigArray_t& data )
{
    size_t count = data.size-1;
    while ( data[count]==0 && count>0 ) {
        count--;
    }
    return (data.size-count)-1;
}

/**
 * Convert from the rawReading to a different sample rate
 *
 * @param output
 * @param newPeriod
 */
void Signature::downSample( SigArray_t * output, const size_t newPeriod )
{
    size_t inner, inputIndex, outputIndex, outerLimit;
    Sample_t accumulator;

    LOG(INFO) << "Resampling...";

    const size_t newSize = (int)ceil(rawReading.size / ( (double)newPeriod / (double)samplePeriod ));
    output->setSize( newSize );


    const size_t mod = newPeriod % samplePeriod;
    if (mod==0) {
        // newPeriod % samplePeriod == 0; so then just take an average of each step
        const size_t stepSize = newPeriod/samplePeriod;
        const size_t delta = (newSize*newPeriod) - (rawReading.size*samplePeriod);

        if (delta==0) {
            outerLimit = newSize;
        } else {
            outerLimit = newSize-1;
        }

        LOG(INFO) << "old size=" << rawReading.size << ", old period=" << samplePeriod
                  << ", new size=" << newSize << ", newPeriod=" << newPeriod
                  << ", stepSize=" << stepSize << ", mod=" << mod << ", delta=" << delta << ",outerLimit=" << outerLimit;


        // Do the vast majority of the resampling
        for (outputIndex=0; outputIndex<outerLimit; outputIndex++) {
            accumulator=0;
            for (inner=0; inner<stepSize; inner++) {
                inputIndex = inner+(outputIndex*stepSize);
                accumulator += rawReading[ inputIndex ];
            }
            (*output)[ outputIndex ] = accumulator/stepSize;
        }

        // Do the remainder (if there is any)
        if (delta != 0) {
            accumulator=0;
            size_t i;
            for (i=inputIndex; i<rawReading.size; i++) {
                accumulator += rawReading[i];
                LOG(INFO) << "i=" << i << ", rawReading[i]=" << rawReading[i];
            }
            (*output)[ newSize-1 ] = accumulator/(delta-1);
            LOG(INFO) << "Filled remainder.  output[" << newSize-1 << "]=" << (*output)[newSize-1];
        }

    } else {
        // newPeriod%samplePeriod!=0 so interpolate
        LOG(ERROR) << "resample() doesn't yet deal with fractional sample rate conversions";

    }


}

const size_t Signature::getSamplePeriod()
{
    return samplePeriod;
}
