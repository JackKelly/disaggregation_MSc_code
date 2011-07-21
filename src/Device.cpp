/*
 * Device.cpp
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#include "Device.h"
#include "Signature.h"
#include "Common.h"
#include <list>
#include <glog/logging.h>
#include <cassert>
#include <cstring>

using namespace std;

Device::Device(const string _name) : name(_name) {
    LOG(INFO) << "Creating new Device: " << name;
}

Device::~Device() {
    for (vector<Signature*>::iterator it; it!=signatures.end(); it++) {
        delete *it;
    }
}

void Device::getReadingFromCSV(const char * filename, const size_t samplePeriod, const size_t cropFront, const size_t cropBack)
{
    // Create a new signature
    Signature * sig = new Signature( filename, samplePeriod, cropFront, cropBack );
    signatures.push_back( sig );

    const size_t rollingAvLength = 31;

    // Update powerstates
    updatePowerStates( rollingAvLength );

    updatePowerStateSequence( rollingAvLength );

    dumpPowerStateSequenceToFile( );
}

/**
 * Extract the power states from last signature and add them to this device's powerStates.
 *
 * This must be called after a new signature has been added to 'signatures'
 */
void Device::updatePowerStates( const size_t rollingAvLength )
{
    // sanity check
    assert( ! signatures.empty() );

    // get power states from last signatures
    powerStates = signatures.back()->getPowerStates( rollingAvLength );

    /* TODO: deal with the case where we already have some powerStates stored
     * and we want to merge the new power states from the new signature with
     * the existing power states.  */
}

/**
 * Take 'signatures.end()' and run through the 'rawReading', finding where
 * each 'powerState' starts and ends.
 *
 * This must be called after 'powerStates' has been populated
 */
void Device::updatePowerStateSequence( const size_t rollingAvLength )
{
    // sanity check
    assert( ! powerStates.empty() );

    RollingAv_t reading;
    signatures.back()->getRawReading().rollingAv( &reading, rollingAvLength);

//    reading = signatures.back()->getRawReading();


    const size_t samplePeriod    = signatures.back()->getSamplePeriod();

    PowerStateSequenceItem powerStateSequenceItem;
    PowerStates_t::const_iterator currentPowerState;

    enum { WITHIN_STATE, NO_MANS_LAND } state;
    state = NO_MANS_LAND;

    // Go through raw reading finding occurences of each power state
    for (size_t i=0; i<reading.size; i++ ) {
        currentPowerState = getPowerState( reading[i] );

        switch (state) {
        case WITHIN_STATE:
            if (currentPowerState != powerStateSequenceItem.powerState) {
                        // then we've hit a powerState transition
                        // so store the details of the powerState we've just left
                        powerStateSequenceItem.endTime   = i*samplePeriod;
                        powerStateSequence.push_back( powerStateSequenceItem ); // save a copy

                        LOG(INFO) << "Power state transition. start=" << powerStateSequenceItem.startTime
                                << "\tendTime=" << powerStateSequenceItem.endTime
                                << "\tduration=" << powerStateSequenceItem.endTime - powerStateSequenceItem.startTime
                                << "\t" << *powerStateSequenceItem.powerState;

                        if ( currentPowerState == powerStates.end() ) {
                            // We've entered an unrecognised state
                            state = NO_MANS_LAND;
                        } else {
                            // We've transitioned directly from one recognised state to another
                            powerStateSequenceItem.powerState = currentPowerState;
                            powerStateSequenceItem.startTime  = i*samplePeriod;
                        }
            }
            break;
        case NO_MANS_LAND:
            if ( currentPowerState != powerStates.end() ) {
                // We've entered a recognised state
                state = WITHIN_STATE;
                powerStateSequenceItem.powerState = currentPowerState;
                powerStateSequenceItem.startTime  = i*samplePeriod;
            }
            break;
        };
    }

    // Handle case where final state is left hanging after for loop
    if ( state == WITHIN_STATE ) {
        powerStateSequenceItem.endTime = reading.size*samplePeriod;
        powerStateSequence.push_back( powerStateSequenceItem ); // save a copy
//        LOG(INFO) << "Power state transition. start=" << powerStateSequenceItem.startTime
//                << "\tendTime=" << powerStateSequenceItem.endTime
//                << "\tduration=" << powerStateSequenceItem.endTime - powerStateSequenceItem.startTime
//                << "\t" << *powerStateSequenceItem.powerState;
    }

    /* TODO: Detect repeats */
}

/**
 * Dump the contents of powerStateSequence to a data file ready for
 * gnuplot to plot using the 'boxxyerrorbars' style (see p42 of the gnuplot 4.4 documentation PDF).
 *
 * @param filename
 */
void Device::dumpPowerStateSequenceToFile( )
{
    // open datafile
    char * filename = (char*)DIAGRAMS_DIR.c_str() ;
    strcat( filename, "powerStateSequence/powerStateSequence.dat" );

    fstream dataFile;
    dataFile.open( filename, fstream::out );
    if ( ! dataFile.good() ) {
        LOG(WARNING) << "Failed to open data file " << filename << " for output";
        return;
    }

    // Output header information for data file
    dataFile << "# automatically produced by dumpPowerStateSequenceToFile function\n"
             << "# " << Utils::todaysDateAndTime() << endl
             << "# x\ty\txlow\txhigh\tylow\tyhigh" << endl;

    // loop through the 'poewStateSequence' list
    for (list<PowerStateSequenceItem>::const_iterator powerState=powerStateSequence.begin();
            powerState!=powerStateSequence.end();
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

/**
 *
 * @param  sample
 * @return pointer to 'powerState' from 'powerStates' corresponding to 'sample'.
 * Returns 'powerStates.end()' if 'sample' does not correspond to any 'powerState'
 */
PowerStates_t::const_iterator Device::getPowerState( const Sample_t sample )
{
    // sanity check
    assert( ! powerStates.empty() );

    // Doing the inelegant, brute-force approach
    for (PowerStates_t::const_iterator powerState = powerStates.begin();
         powerState != powerStates.end();
         powerState++) {

        if ( Utils::roundToNearestInt(sample) <= powerState->max &&
             Utils::roundToNearestInt(sample) >= powerState->min ) {
            return powerState;
        }
    }

    // If we get to here then 'sample' does not fall within any powerState boundary
//    LOG(WARNING) << "Couldn't find sample within powerStates.  sample=" << sample;
    return powerStates.end();
}

list<size_t> Device::findAlignment( const char * aggregateDataFilename )
{
    LOG(INFO) << "Attempting to find location of " << name << " in aggregate data file " << aggregateDataFilename;

    fstream aggregateDataFile;
    Utils::openFile( aggregateDataFile, "data/current_cost/dataCroppedToKettleToasterWasherTumble.csv", fstream::in );

    Array<size_t> aggregateData;
    loadCurrentCostData( &aggregateData, aggregateDataFile );
}

/**
 * TODO: This currently doesn't handle the fact that the CurrentCost drops a reading every now and then
 *
 * @param aggregateData
 * @param aggregateDataFile
 */
void Device::loadCurrentCostData( Array<size_t> * aggregateData, fstream& aggregateDataFile )
{
    assert( aggregateData );
    aggregateData->setSize( Utils::countDataPoints( aggregateDataFile ) );

    int count = 0;
    char ch;
    while ( ! aggregateDataFile.eof() ) {
        ch = aggregateDataFile.peek();
        if ( isdigit(ch) ) {
            aggregateDataFile >> (*aggregateData)[ count++ ];  // attempt to read a num from the file
        }
        aggregateDataFile.ignore( 255, '\n' );  // skip to next line
    }
    LOG(INFO) << "Entered " << count << " data entries into data array.";
}

