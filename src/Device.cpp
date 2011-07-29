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

    updatePowerStates();

    updatePowerStateSequence();

    getSalientSpikes();
}

/**
 * @brief Extract the power states from last signature and add them to this device's powerStates.
 *
 * This must be called after a new signature has been added to 'signatures'
 */
void Device::updatePowerStates()
{
    // sanity check
    assert( ! signatures.empty() );

    // get power states from last signatures
    powerStates = signatures.back()->getPowerStates();

    /**
     * @todo deal with the case where we already have some powerStates stored
     * and we want to merge the new power states from the new signature with
     * the existing power states.  */
}

/**
 * @brief Extract the power state sequence from last signature and add them to this device's powerStates.
 *
 * This must be called after a new signature has been added to 'signatures'
 */
void Device::updatePowerStateSequence()
{
    // sanity check
    assert( ! signatures.empty() );

    // get power states from last signatures
    powerStateSequence = signatures.back()->getPowerStateSequence();

    powerStateSequence.dumpToFile( name );

    /**
     * @todo deal with the case where we already have a powerStateSequence stored
     * and we want to merge the new power state sequence from the new signature with
     * the existing power states.  */
}

/**
 * @return a list of the most salient spikes to look for,
 *         taking into consideration all Signatures,
 *         in ascending temporal order.
 */
const list<Signature::Spike> Device::getSalientSpikes()
{
    // get the gradient spikes for the first signature
    list<Signature::Spike> spikes = signatures.front()->getGradientSpikesInOrder();

    // take just the top ten (by absolute value)
    if (spikes.size() > 10) {
        list<Signature::Spike>::iterator it = spikes.begin();
        advance( it, 10 );
        spikes.erase( it, spikes.end() );
    }

    // re-order by index (i.e. by time)
    spikes.sort( Signature::Spike::compareIndexAsc );

    /**
     * @todo if there are multiple signatures then do this:
     *       1) getGradientSpikesInOrder() for sig1. Don't throw any away.
     *       2) attempt to find these spikes subsequent sigs.  If a spike can't be find in a subsequent sig then remove it.
     *       3) order the new set and return.
     */

    // just for diagnostics:
    cout << "Salient spikes: \n"
            "index\tvalue\tduration" << endl;
    for(list<Signature::Spike>::const_iterator it=spikes.begin(); it!=spikes.end(); it++)
        cout << it->index << "\t" << it->value << "\t" << it->duration << endl;

    return spikes;
}

/**
 * @todo this doesn't belong in Device
 */
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
 *   @todo this doesn't belong in Device
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

