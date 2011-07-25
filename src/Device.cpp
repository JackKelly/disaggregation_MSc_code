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
#include <cstdlib> // abs

using namespace std;

Device::Device(const string _name) : name(_name) {
    LOG(INFO) << "Creating new Device: " << name;
}

Device::~Device() {
    for (vector<Signature*>::iterator it; it!=signatures.end(); it++) {
        delete *it;
    }
}

const string Device::getName() const
{
    return name;
}

void Device::getReadingFromCSV(const char * filename, const size_t samplePeriod, const size_t cropFront, const size_t cropBack)
{
    // Create a new signature
    Signature * sig = new Signature(
            filename, samplePeriod,
            name,
            signatures.size(), // Provides the signature with its sigID.
            cropFront, cropBack
            );
    signatures.push_back( sig );

    const size_t rollingAvLength = 31;

    // Update powerstates
    updatePowerStates( rollingAvLength );

    updatePowerStateSequence( rollingAvLength );

    dumpPowerStateSequenceToFile( );
}

/**
 * @brief Extract the power states from last signature and add them to this device's powerStates.
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
 * Run through 'signatures.end()->data' , finding where
 * each 'powerState' starts and ends.
 *
 * This must be called after 'powerStates' has been populated
 */
void Device::updatePowerStateSequence( const size_t rollingAvLength )
{
    // sanity check
    assert( ! powerStates.empty() );

    Array<Sample_t> reading;
    signatures.back()->rollingAv( &reading, rollingAvLength);

    const size_t samplePeriod    = signatures.back()->getSamplePeriod();

    PowerStateSequenceItem powerStateSequenceItem;
    PowerStates_t::const_iterator currentPowerState;

    enum { WITHIN_STATE, NO_MANS_LAND } state;
    state = NO_MANS_LAND;

    // Go through raw reading finding occurences of each power state
    for (size_t i=0; i<reading.getSize(); i++ ) {
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
        powerStateSequenceItem.endTime = reading.getSize()*samplePeriod;
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
    char * filename = (char*)DATA_OUTPUT_PATH.c_str() ;
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

list<size_t> Device::findAlignment( const char * aggregateDataFilename, const size_t aggDataSamplePeriod )
{
    const double THRESHOLD = 250;

    LOG(INFO) << "Attempting to find location of " << name << " in aggregate data file " << aggregateDataFilename;

    // Load aggregate data file
    fstream aggregateDataFile;
    Utils::openFile( aggregateDataFile, aggregateDataFilename, fstream::in );

    list<size_t> locations;

    Array<currentCostReading> aggregateData;
    loadCurrentCostData( aggregateDataFile, &aggregateData );
    aggregateDataFile.close();

    // Get a handy reference to the last 'rawReading' stored in 'signatures'
    const Array<Sample_t>& sigArray = *(signatures.back());

    double min = 100000000000000000;

    double lms;
    size_t foundAt;
    size_t i;
    for (i = 0;
            i < aggregateData.getSize() - (sigArray.getSize() / (aggDataSamplePeriod / signatures.back()->getSamplePeriod() ));
            i++) {
        lms = LMDiff(i, aggregateData, sigArray, aggDataSamplePeriod);
        if ( lms < THRESHOLD ) {
            locations.push_back( i );
            LOG(INFO) << name << " found at" << aggregateData[i].timestamp - aggregateData[0].timestamp << " lmS=" << lms;
        }

        if ( lms < min ) {
            min = lms;
            foundAt = aggregateData[i].timestamp - aggregateData[0].timestamp;
        }
    }

    LOG(INFO) << "aggregateData[0].timestamp=" << aggregateData[0].timestamp
            << ", aggregateData[0].reading=" << aggregateData[0].reading;
    LOG(INFO) << name << " found at " << foundAt << ", LMS=" << min;

    LOG(INFO) << "length=" << i*aggDataSamplePeriod;

    LOG(INFO) << "min=" << min;

    return locations;
}

/**
 * Least Mean Difference (like LMS but take the absolute rather than the square).
 *
 * Know limitations:
 *   assumes SigArray has a sample period of 1
 *   doesn't try different alignments of sigArray against aggData
 *
 * @return
 */
const double Device::LMDiff(
        const size_t aggOffset,
        const Array<currentCostReading>& aggData, /**< aggregate data array */
        const Array<Sample_t>& sigArray,
        const size_t aggDataSamplePeriod /**< sample period of the aggregate data */
        )
{
    double accumulator = 0;

// THIS COMMENTED-OUT CODE ASSUMES THERE ARE NO MISSING SAMPLES.  I'M KEEPING IT HERE FOR NOW
// BECAUSE IT'S USEFUL FOR TESTING BY FINDING A CROPPED DEVICE SIGNATURE WITHIN ITSELF
//
//    for (i=0; i<((sigArray.getSize()/aggDataSamplePeriod)-1); i++) {
//        for (size_t fineTune=0; fineTune<aggDataSamplePeriod; fineTune++) {
//            accumulator += abs( sigArray[(i*aggDataSamplePeriod)+fineTune] - aggData[i+agOffset] );
//        }
//        accumulator += pow( ( sigArray[(i*aggDataSamplePeriod)] - aggData[i+agOffset] ) ,2);
//    }
//    LOG(INFO) << "i*aggDataSample=" << i*aggDataSamplePeriod << "sigArray.getSize()=" << sigArray.getSize();

    size_t aggIndex=aggOffset, sigIndex=0, count=0;

    // Take the first few readings to establish the level difference


// COMPARE START OF AGG DATA WITH START OF SIG DATA
    while (sigIndex < (sigArray.getSize()-aggDataSamplePeriod) && aggIndex < (aggData.getSize()-1) && count < 50) {
        accumulator += aggData[aggIndex].reading - sigArray[sigIndex]; // don't take the absolute
        count++;
        aggIndex++;
        // Inc sigIndex by however many seconds are between this and the previous aggData recording
        sigIndex += (aggData[aggIndex].timestamp - aggData[aggIndex-1].timestamp);
    }

    /*
    while (sigIndex < (aggIndex < (aggData.getSize()-1)) && count < 50) {
        accumulator += aggData[aggIndex].reading; // don't take the absolute
        count++;
        aggIndex++;
    }*/

    int levelDiff = accumulator/count; // -ve if sig is above aggregate
//    LOG(INFO) << "levelDiff=" << levelDiff;

    accumulator = 0;
    aggIndex=aggOffset;
    sigIndex=0;
    count=0;
    while (sigIndex < (sigArray.getSize()-aggDataSamplePeriod) && aggIndex < (aggData.getSize()-1)) {
            for (size_t fineTune = 0; fineTune<aggDataSamplePeriod; fineTune++) {
                accumulator += abs( (sigArray[sigIndex+fineTune]+levelDiff) - aggData[aggIndex].reading );
                count++;
            }
            aggIndex++;
            // Inc sigIndex by however many seconds are between this and the previous aggData recording
            sigIndex += (aggData[aggIndex].timestamp - aggData[aggIndex-1].timestamp);
    }

//    LOG(INFO) << "Offset=" << agOffset << ", LMDiff=" << accumulator / (sigArray.getSize()/aggDataSamplePeriod);

    return accumulator / count; // average
}

/**
 * @todo why are we loading current cost data in Device?
 * @param fs
 * @param data = a pointer to a valid but empty Array<Sample_t>
 */
void Device::loadCurrentCostData(std::fstream& fs, Array<currentCostReading> * aggData)
{
    aggData->setSize( Utils::countDataPoints( fs ) );

    int count = 0;
    char ch;
    while ( ! fs.eof() ) {
        ch = fs.peek();
        if ( isdigit(ch) ) {
            fs >> (*aggData)[ count ].timestamp;
            fs >> (*aggData)[ count ].reading;
            count++;
        }
        fs.ignore( 255, '\n' );  // skip to next line
    }
    LOG(INFO) << "Entered " << count << " ints into data array.";
}

