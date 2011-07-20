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
    PowerStates_t::const_iterator currentPowerState;



    // Find start of first powerState
/*
       size_t i=0;
       do {
        powerStateSequenceItem.powerState = getPowerState( rawReading[i] );
        powerStateSequenceItem.startTime  = i*samplePeriod;
        i++;
        assert ( i != rawReading.size );
    } while ( powerStateSequenceItem.powerState == powerStates.end() );
  */
    enum { WITHIN_STATE, NO_MANS_LAND } state;
    state = NO_MANS_LAND;

    // Go through raw reading finding occurences of each power state
    for (size_t i=0; i<rawReading.size; i++ ) {
        currentPowerState = getPowerState( rawReading[i] );

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
        powerStateSequenceItem.endTime = rawReading.size*samplePeriod;
        powerStateSequence.push_back( powerStateSequenceItem ); // save a copy
        LOG(INFO) << "Power state transition. start=" << powerStateSequenceItem.startTime
                << "\tendTime=" << powerStateSequenceItem.endTime
                << "\tduration=" << powerStateSequenceItem.endTime - powerStateSequenceItem.startTime
                << "\t" << *powerStateSequenceItem.powerState;
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
//    LOG(WARNING) << "Couldn't find sample within powerStates.  sample=" << sample;
    return powerStates.end();
}
