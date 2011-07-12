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

/**
 * Runs through 'rawReading' to find power states.  'rawReading' must be
 * populated before this function is called.
 */
void Signature::findPowerStates()
{
    LOG(INFO) << "Finding power states...";
    assert ( rawReading.size ); // make sure rawReading is populated

    // Create a histogram
    HistogramArray_t hist;
    rawReading.histogram( &hist );

    // Take a 21-stage rolling av
    RollingAv_t histRA;
    hist.rollingAv( &histRA , 31 );

    histRA.dumpToFile("histRA.csv");

    // histRA now contains a smoothed histogram of 'rawReading'

    Sample_t maxValue;
    size_t   indexOfMaxValue, start, end;
    list<size_t> stateBoundaries;
    /* Now loop through the smoothed histogram to find the positions
     * of each peak
     * Loop until size of max is less than, say, 5% of rawReading.size */
    do {
        //    Find max (masked by list of peaks)
        indexOfMaxValue = histRA.max( &maxValue, stateBoundaries );

        //    Descend peak
        histRA.descendPeak( indexOfMaxValue, &start, &end, 15);

        //    Find stats for peak and append to powerStates
        powerStates.push_back( Statistic<Histogram_t>( hist, start, end ) );

        //    Append start and end of peak to list
        stateBoundaries.push_back( start );
        stateBoundaries.push_back( end   );

        LOG(INFO) << "Power state found. maxVal=" << maxValue << " indexOfMax=" << indexOfMaxValue << " Start=" << start << " end=" << end << " stats: " << powerStates.back();

    } while ( maxValue > (0.002*rawReading.size) );


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

const SigArray_t& Signature::getSigArray() const
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

