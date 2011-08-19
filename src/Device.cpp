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
#include <cassert>
#include <cstring>
#include <cstdlib> // abs
#include <limits> // std::numeric_limits<std::size_t>::max

using namespace std;

Device::Device(const string _name) : name(_name) {
    cout << "Creating Device object with name: " << name << endl;
}

Device::~Device() {
    for (vector<Signature*>::iterator it=signatures.begin(); it!=signatures.end(); it++) {
        delete *it;
    }
}

const string Device::getName() const
{
    return name;
}

void Device::loadSignatures(
        const vector< string >& sigFiles,
        const size_t cropFront,
        const size_t cropBack
        )
{
    vector< string >::const_iterator sigFile;
    for (sigFile = sigFiles.begin(); sigFile!=sigFiles.end(); sigFile++) {

        // Check sig file exists
        string fullSigFilename = SIG_DATA_PATH + *sigFile;
        if ( ! Utils::fileExists( fullSigFilename ) ) {
            Utils::fatalError( "Signature file " + fullSigFilename + " does not exist." );
        }

        // Create a new signature
        Signature * sig = new Signature
                (
                fullSigFilename,  // filename
                1,                // sample period
                sigFile->substr(0, sigFile->length()-4), // sig filename without suffix
                signatures.size(),// Provides the signature with its sigID.
                cropFront, cropBack
                );

        signatures.push_back( sig );
    }
}

/**
 * @brief To be called once @c signatures have been loaded
 */
void Device::trainPowerStateGraph()
{
    vector< Signature* >::const_iterator sig;
    for (sig = signatures.begin(); sig!=signatures.end(); sig++) {
        powerStateGraph.update( *(*sig) );
    }

    cout << endl
         << "Power State Graph vertices:" << endl
         << powerStateGraph << endl;

    // output power state graph to file
    fstream fs;
    const string psgFilename = DATA_OUTPUT_PATH + "powerStateGraph.gv";
    cout << "Outputting power state graph to " << psgFilename << endl;
    Utils::openFile(fs, psgFilename, fstream::out);
    powerStateGraph.writeGraphViz( fs );
    fs.close();
}

PowerStateGraph& Device::getPowerStateGraph()
{
    return powerStateGraph;
}

/**
 * @deprecated
 */
void Device::getReadingFromCSV(
        const char * filename,
        const size_t samplePeriod,
        const size_t cropFront,
        const size_t cropBack
        )
{
    // Create a new signature
    Signature * sig = new Signature(
            filename, samplePeriod,
            name,
            signatures.size(), // Provides the signature with its sigID.
            cropFront, cropBack
            );
    signatures.push_back( sig );

    updatePowerStates();  // old technique

    updatePowerStateSequence();  // old technique

    getSalientSpikes();  // old technique
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
const list<Signature::Spike> Device::getSalientSpikes() const
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
/*    cout << "Salient spikes: \n"
            "index\tvalue\tduration" << endl;
    for(list<Signature::Spike>::const_iterator it=spikes.begin(); it!=spikes.end(); it++)
        cout << it->index << "\t" << it->value << "\t" << it->duration << endl;
*/

    return spikes;
}

/**
 *
 * @return a list of UNIX times when the device starts
 *
 * @deprecated We now use the disaggregation functions in PowerStateGraph
 */
const list<size_t> Device::getStartTimes(
        const AggregateData& aggregateData /**< A populated array of AggregateData */
        ) const
{
    cout << "Getting start times for " << name << endl;

    /** @todo should be relative */
    const size_t DIFF_THRESHOLD = 50; // in Watts.  Difference between the size spike in the device signature and in the aggregate data.

    IndexAndDelta indexAndDelta;
    list<size_t> startTimes;
    list<IndexAndDelta> candidates;

    const list<Signature::Spike> spikes = getSalientSpikes();

    // try to find the first spike from 'spikes' in the aggregateData
    for (indexAndDelta.index = 0; indexAndDelta.index < aggregateData.getSize(); indexAndDelta.index++) {

        indexAndDelta.delta =
                abs(aggregateData.aggDelta( indexAndDelta.index ) - spikes.front().delta);

        if (indexAndDelta.delta < DIFF_THRESHOLD) {
            candidates.push_back( indexAndDelta );
            cout << indexAndDelta.index << "\t"
                    << aggregateData.secondsSinceFirstSample(indexAndDelta.index) << "\t" << indexAndDelta.delta << endl;
        }
    }

    // now try to find each of the other device signature spikes
    // where we'd expect to find them relative to the first spike
    size_t expectedDistance, actualDistance;
    list<Signature::Spike>::const_iterator spike;
    for (list<IndexAndDelta>::iterator candidate=candidates.begin();
            candidate!=candidates.end();
            candidate++) {

        spike = spikes.begin();
        advance( spike, 1 );
        for (; spike!=spikes.end(); spike++) {
            expectedDistance = spike->index - spikes.front().index;

            actualDistance = aggregateData.findNear(
                    candidate->index, expectedDistance, spike->delta );

        }

    }

    return startTimes;
}

/**
 * @todo this doesn't belong in Device - maybe better in Signature or AggregateData?
 */
list<size_t> Device::findAlignment(
        const AggregateData& aggData
        )
{
    cout << endl
         << "Attempting to find location of " << name << " in aggregate data." << endl;

    list<size_t> locations;

    // Get a handy reference to the last 'rawReading' stored in 'signatures'
    const Array<Sample_t>& sigArray = *(signatures.back());

    double min = numeric_limits<double>::max();

    double lms;
    size_t foundAt=0;
    const size_t end =
            aggData.getSize() -
                (sigArray.getSize() /
                    (aggData.getSamplePeriod() / signatures.back()->getSamplePeriod() ));
    size_t i;
    for (i = 0; i < end; i++) {
        lms = LMS(i, aggData, sigArray, aggData.getSamplePeriod());
        if ( lms < min ) {
            min = lms;
            foundAt = aggData[i].timestamp;
        }
    }

    cout << name << " found at " << foundAt << ", LMS=" << min << endl;

    cout << "length=" << i*aggData.getSamplePeriod() << endl;

    cout << "min=" << min << endl;

    return locations;
}

/**
 * @brief Least mean difference
 *
 * Know limitations:
 *   assumes SigArray has a sample period of 1
 *
 *   @todo this doesn't belong in Device
 *
 * @return
 */
const double Device::LMS(
        const size_t aggOffset,
        const Array<AggregateSample>& aggData, /**< aggregate data array */
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
//    cout << "i*aggDataSample=" << i*aggDataSamplePeriod << "sigArray.getSize()=" << sigArray.getSize() << endl;

    size_t aggIndex=aggOffset, sigIndex=0, count=0;

    // Take the first few readings to establish the level difference


    // COMPARE START OF AGG DATA WITH START OF SIG DATA
    while (sigIndex < (sigArray.getSize()-aggDataSamplePeriod)
            && aggIndex < (aggData.getSize()-1) && count < 50) {
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

    accumulator = 0;
    aggIndex=aggOffset;
    sigIndex=0;
    count=0;
    double diff;

    bool printThis = (aggData[aggIndex].timestamp==1310294457);

    while (sigIndex < (sigArray.getSize()-aggDataSamplePeriod) && aggIndex < (aggData.getSize()-1)) {
            for (size_t fineTune = 0; fineTune<aggDataSamplePeriod; fineTune++) {
                diff = (sigArray[sigIndex+fineTune]+levelDiff) - aggData[aggIndex].reading;
                accumulator += pow(diff,2);

                if (printThis) {
                    cout.setf(ios::fixed);
                    cout.precision(2);
                    cout << (aggData[aggIndex].timestamp + fineTune) << "\t" << pow(diff,2) << endl;
                }

                count++;
            }
            aggIndex++;
            // Inc sigIndex by however many seconds are between this and the previous aggData recording
            sigIndex += (aggData[aggIndex].timestamp - aggData[aggIndex-1].timestamp);
    }

//    cout << "Offset=" << agOffset << ", LMDiff=" << accumulator / (sigArray.getSize()/aggDataSamplePeriod) << endl;

    return accumulator / count; // average
}

