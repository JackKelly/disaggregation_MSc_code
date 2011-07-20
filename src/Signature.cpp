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
Signature::Signature(const char* filename, const size_t _samplePeriod)
: samplePeriod(_samplePeriod)
{
    ifstream dataFile;
    openFile( dataFile, filename );

    SigArray_t data;
    loadData( dataFile, &data );
    dataFile.close();

    cropAndStore( data );
}

Signature::~Signature()
{}

const PowerStates_t& Signature::getPowerStates()
{
    if ( powerStates.empty() ) {
        findPowerStates();
    }

    return powerStates;
}

void Signature::drawGraphWithStateBars( const HistogramArray_t& hist )
{
    // dump data to temp file
    hist.dumpToFile( "hist.dat" );

    const std::string name("hist");

    GNUplot gnu_plot;

    gnu_plot( "set title \"" + name + "\"" );
    gnu_plot( "set terminal svg size 600 500" );
    gnu_plot( "set samples 1001" ); // high quality
    gnu_plot( "set output \"" + name + ".svg\"" );
    gnu_plot( "set xlabel \"power (Watts)\"" );
    gnu_plot( "set ylabel \"frequency\"" );
    gnu_plot( "plot [0:2500]  "
              "\"hist.dat\" with l lw 1 title \"raw histogram\", "
              "\"RA_hist.dat\" with l lw 1 title \"31-step rolling average of histogram gradient\", "
              "\"x_err_bars.dat\" with xerrorbars title \"automatically determined state boundaries (min, mean, max)\" " );

    // output for LaTeX
    gnu_plot( "set format \"$%g$\"" );
    gnu_plot( "set terminal epslatex colour solid size 15cm, 10cm" );
    gnu_plot( "set output \"" + name + ".tex\"" );
    gnu_plot( "plot [0:2400] "
              "\"hist.dat\" with l lw 1 title \"raw histogram\", "
              "\"RA_hist.dat\" with l lw 1 title \"31-step rolling average of histogram gradient\", "
              "\"x_err_bars.dat\" with xerrorbars title \"automatically determined state boundaries (min, mean, max)\" " );

}

/**
 * Runs through 'rawReading' to find power states.  'rawReading' must be
 * populated before this function is called.
 */
void Signature::findPowerStates()
{
    LOG(INFO) << "Finding power states...";
    assert ( rawReading.size ); // make sure rawReading is populated

    rawReading.drawGraph( "rawReading", "time (seconds)", "power (Watts)", "[] []" );

/*    SigArray_t downSampled;
    downSample( &downSampled, 6);
    downSampled.drawGraph( "DownSampled6", "time (seconds)", "power (Watts)", "[] []" );
*/

    // Create a histogram
    HistogramArray_t hist;
    rawReading.histogram( &hist );
    hist.drawGraph( "Hist_from_raw", "power (Watts)", "frequency", "[0:2500] [0:100]" );

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
    dataFile.open( "x_err_bars.dat", fstream::out );
    for (std::list<size_t>::const_iterator it=boundaries.begin(); it!=boundaries.end(); it++) {

        front = *it;
        back  = *(++it);

        powerStates.push_back( PowerState_t( hist, front, back ) );

        std::cout << powerStates.back() << std::endl;

        powerStates.back().xErrorBarOutputLine( dataFile );
    }
    dataFile.close();
    drawGraphWithStateBars( hist );

    fillGapsInPowerStates( hist );
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

void Signature::openFile(ifstream& fs, const char* filename)
{
    fs.open( filename, ifstream::in );
    if ( ! fs.good() ) {
        LOG(ERROR) << "Failed to open " << filename << " for reading.";
        exit(EXIT_ERROR);
    }
    LOG(INFO) << "Successfully opened " << filename;
}

/**
 *
 * @param fs
 * @param data = a pointer to a valid but empty SigArray_t
 */
void Signature::loadData(ifstream& fs, SigArray_t* data)
{
    assert( data );
    data->setSize( countDataPoints( fs ) );

    int count = 0;
    char ch;
    while ( ! fs.eof() ) {
        ch = fs.peek();
        if ( isdigit(ch) ) {
            fs >> (*data)[ count++ ];  // attempt to read a float from the file
        }
        fs.ignore( 255, '\n' );  // skip to next line
    }
    LOG(INFO) << "Entered " << count << " ints into data array.";
}

const SigArray_t& Signature::getRawReading() const
{
    return rawReading;
}

/**
 * Find the number of numeric data points in file
 *
 * @param fs
 * @return number of data points
 */
const size_t Signature::countDataPoints( ifstream& fs ) const
{
    int count = 0;
    char line[255];

    while ( ! fs.eof() ) {
        fs.getline( line, 255 );
        if ( isdigit(line[0]) ) {
            count++;
        }
    }

    LOG(INFO) << "Found " << count << " data points in file.";

    // Return the read pointer to the beginning of the file
    fs.clear();
    fs.seekg( 0, ios_base::beg ); // seek to beginning of file

    return count;
}

/**
 * Remove leading zeros and trailing zeros from 'data'
 *
 * @param data
 */
void Signature::cropAndStore( const SigArray_t& data )
{
    const size_t numLeadingZeros  = findNumLeadingZeros( data );
    const size_t numTrailingZeros = findNumTrailingZeros( data );
    LOG(INFO) << numLeadingZeros << " leading zero(s) and " << numTrailingZeros << " trailing zero(s) found.";

    rawReading.copyCrop( data, numLeadingZeros, numTrailingZeros );
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
