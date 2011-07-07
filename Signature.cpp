/*
 * Signature.cpp
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#include "Signature.h"
#include "Common.h"
#include <glog/logging.h>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>

using namespace std;

/**
 * Constructor for opening a CSV file
 *
 */
Signature::Signature(const char* filename, const int _samplePeriod)
: samplePeriod(_samplePeriod)
{
    ifstream dataFile;
    openFile( dataFile, filename );

    SigArray data;
    loadData( dataFile, &data );
    dataFile.close();

    cropAndStore( data );
}

Signature::~Signature()
{}

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
 * @param data = a pointer to a valid but empty SigArray
 */
void Signature::loadData(ifstream& fs, SigArray* data)
{
    assert( data );
    const int numDataPoints =  countDataPoints( fs );
    data->setSize( numDataPoints );

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

/**
 * Find the number of numeric data points in file
 *
 * @param fs
 * @return number of data points
 */
int Signature::countDataPoints( ifstream& fs )
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

int Signature::roundToNearestInt( const double input )
{
    return floor( input + 0.5 );
}

void Signature::cropAndStore( const SigArray& data )
{
    const int numLeadingZeros  = findNumLeadingZeros( data );
    const int numTrailingZeros = findNumTrailingZeros( data );
    LOG(INFO) << numLeadingZeros << " leading zero(s) and " << numTrailingZeros << " trailing zero(s) found.";

    rawReading.copyCrop( data, numLeadingZeros, numTrailingZeros );
}

const int Signature::findNumLeadingZeros( const SigArray& data )
{
    int count = 0;
    while ( data[count]==0 && count<data.size ) {
        count++;
    }
    return count;
}

const int Signature::findNumTrailingZeros( const SigArray& data )
{
    int count = data.size-1;
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
void Signature::downSample( SigArray& output, const int newPeriod )
{
    int inner, inputIndex, outputIndex, outerLimit;
    SigArrayDataType accumulator;

    LOG(INFO) << "Resampling...";

    const int newSize = (int)ceil(rawReading.size / ( (double)newPeriod / (double)samplePeriod ));
    output.setSize( newSize );


    const int mod = newPeriod % samplePeriod;
    if (mod==0) {
        // newPeriod % samplePeriod == 0; so then just take an average of each step
        const int stepSize = newPeriod/samplePeriod;
        const int delta = (newSize*newPeriod) - (rawReading.size*samplePeriod);

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
            output[ outputIndex ] = accumulator/stepSize;
        }

        // Do the remainder (if there is any)
        if (delta != 0) {
            accumulator=0;
            int i;
            for (i=inputIndex; i<rawReading.size; i++) {
                accumulator += rawReading[i];
                LOG(INFO) << "i=" << i << ", rawReading[i]=" << rawReading[i];
            }
            output[ newSize-1 ] = accumulator/(delta-1);
            LOG(INFO) << "Filled remainder.  output[" << newSize-1 << "]=" << output[newSize-1];
        }

    } else {
        // newPeriod%samplePeriod!=0 so interpolate
        LOG(ERROR) << "resample() doesn't yet deal with fractional sample rate conversions";

    }


}

