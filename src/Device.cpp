/*
 * Device.cpp
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#include "Device.h"
#include "Signature.h"
#include <list>
#include <glog/logging.h>
#include <cassert>

using namespace std;

Device::Device(const string _name) : name(_name) {
    LOG(INFO) << "Creating new Device: " << name;
}

Device::~Device() {
    for (vector<Signature*>::iterator it; it!=signatures.end(); it++) {
        delete *it;
    }
}

void Device::getReadingFromCSV(const char * filename, const size_t samplePeriod)
{
    // Create a new signature
    Signature * sig = new Signature( filename, samplePeriod );
    signatures.push_back( sig );

    // Update powerstates
    updatePowerStates();

    updatePowerStateSequence();
}

/**
 * Extract the power states from last signature and add them to this device's powerStates.
 *
 * This must be called after a new signature has been added to 'signatures'
 */
void Device::updatePowerStates()
{
    // sanity check
    assert( ! signatures.empty() );

    // get power states from last signatures
    powerStates = signatures.back()->getPowerStates();

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
void Device::updatePowerStateSequence()
{
    // sanity check
    assert( ! powerStates.empty() );

    const SigArray_t& rawReading = signatures.back()->getRawReading();
    const size_t samplePeriod    = signatures.back()->getSamplePeriod();

    PowerStateSequenceItem powerStateSequenceItem;
    PowerStates_t::const_iterator prevPowerState, currentPowerState;

    // Process first rawReading entry
    prevPowerState = getPowerState( rawReading[0] );
    size_t duration = 0;

    // Go through raw reading, occurences of each power state
    for ( size_t i = 1; i<rawReading.size; i++ ) {
        currentPowerState = getPowerState( rawReading[i] );

        assert( currentPowerState != powerStates.end() );

        if (currentPowerState == prevPowerState) {
            duration++;
        } else {
            // then we've hit a powerState transition
            powerStateSequenceItem.powerState = prevPowerState;
            powerStateSequenceItem.duration   = duration*samplePeriod;
            LOG(INFO) << "Power state sequence item: " << *currentPowerState << "\tduration=" << duration*samplePeriod;
            powerStateSequence.push_back( powerStateSequenceItem ); // save a copy
            duration = 1;
        }

        prevPowerState = currentPowerState;
    }

    // Handle case where final state is left hanging after for loop
    if ( duration != 0 ) {
        powerStateSequenceItem.powerState = prevPowerState;
        powerStateSequenceItem.duration   = duration*samplePeriod;
        LOG(INFO) << "Power state sequence item: " << *currentPowerState << "\tduration=" << duration*samplePeriod;
        powerStateSequence.push_back( powerStateSequenceItem ); // save a copy
    }

    /* TODO: Detect repeats */
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
    // this should never happen because powerStates should cover the entire range
    LOG(WARNING) << "Couldn't find sample within powerStates.  sample=" << sample;
    return powerStates.end();
}
