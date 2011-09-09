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
#include "Device.h"
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <list>
#include <algorithm> // for find

using namespace std;

/**
 * @brief Constructor for opening a CSV file.
 *
 * @todo sample period should be read from data file.
 */
Signature::Signature(
        const string& filename,       /**< Filename, including path and suffix. */
        const size_t _samplePeriod, /**< Sample period. In seconds.           */
        const string _deviceName,   /**< All signatures are associated with devices */
        const size_t _sigID,        /**< Each Device can have multiple signatures.
                                         A Device's first sig gets a sigID of 0,
                                         the next gets a sigID of 1 etc. Default=0. */
        const size_t cropFront,     /**< Number of elements to crop from the front.
                                         Default=0. If 0 then will automatically
                                         crop 0s from the front such that there's
                                         only a single leading zero left. */
        const size_t cropBack       /**< Number of elements to crop from the back.
                                         Default=0. If 0 then will automatically
                                         crop 0s from the back such that there's
                                         only a single trailing zero left. */
        )
: samplePeriod(_samplePeriod), sigID(_sigID)
{
    fstream dataFile;
    Utils::openFile( dataFile, filename, fstream::in );

    Array<Sample_t> data;
    data.loadData( dataFile );
    dataFile.close();

    size_t newCropFront = cropFront ? cropFront : data.getNumLeadingZeros();
    size_t newCropBack  = cropBack  ? cropBack  : data.getNumTrailingZeros();

    copyCrop( data, newCropFront, newCropBack );

    deviceName =_deviceName;
    powerStateSequence.setDeviceName( deviceName );

    // Draw graph of raw data after cropping
    drawGraph( "-afterCropping" );

    Array<Sample_t> delta;
    getDelta( &delta );
    delta.drawGraph( "-delta" );

}

Signature::~Signature()
{}


void Signature::drawGraph(
        const string details /**< Type of graph e.g. "gradient".
                                  NOT device name, which gets added automatically. */
        ) const
{
    Array<Sample_t>::drawGraph(
            details,
            "time (seconds)",
            "power (kW)"
            );
}

void Signature::drawHistWithStateBars(
        const Histogram& hist /**< Histogram array. */
    ) const
{
    // Dump histogram data to a .dat file
    hist.dumpToFile( hist.getBaseFilename() );

    // Set plot variables
    GNUplot::PlotVars pv;
    pv.inFilename  = "histWithStateBars";
    pv.outFilename = deviceName + "-histWithStateBars";
    pv.title       = deviceName + " histogram with state bars.";
    pv.xlabel      = "power (Watts)";
    pv.ylabel      = "histogram frequency";

    // Set data for plotting
    pv.data.push_back( GNUplot::PlotData(
            hist.getBaseFilename(),
            "Histogram. HistSmoothing = " + Utils::size_t_to_s( hist.getSmoothing() ) +
                 ". UpstreamSmoothing=" + Utils::size_t_to_s( hist.getUpstreamSmoothing() ),
            "HIST" )
    );

    pv.data.push_back( GNUplot::PlotData (
            hist.getSmoothedGradOfHistFilename(),
            "Gradient of histogram. Gradient smoothing=" + Utils::size_t_to_s( hist.HIST_GRADIENT_RA_LENGTH ),
            "HISTGRAD" )
    );

    pv.data.push_back( GNUplot::PlotData(
            getStateBarsBaseFilename(),
            "Automatically determined power states (min, mean, max)",
            "STATES" )
    );

    // Plot
    GNUplot::plot( pv );
}


const PowerStates_t& Signature::getPowerStates()
{
    if ( powerStates.empty() )
        updatePowerStates();

    return powerStates;
}

/**
 * @brief Runs through signature data to find power states
 *        and stores the resulting power states in powerStates
 *
 * Prerequisites:
 *    signature data must be populated before this function is called.
 */
void Signature::updatePowerStates()
{
    assert ( size ); // make sure Signature is populated

    // Smooth raw data
    Array<Sample_t> RA; // array to hold rolling average
    rollingAv( &RA, PREPROCESSING_DATA_SMOOTHING );
    RA.drawGraph( "", "time (seconds)", "power (Watts)", "" ); /**< @todo drawGraph shouldn't need all these params, surely? */

    // Create histogram from smoothed data
    Histogram hist ( RA );
    hist.drawGraph();

    // find the boundaries of the different power states in the histogram
    std::list<size_t> boundaries;
    hist.findPeaks( &boundaries );

    // calculate proper stats for power states...
    // Go through each pair of boundaries, working out stats for each
    assert( (boundaries.size() % 2)==0 ); // check there's an even number of entries
    size_t front, back;
    fstream dataFile;
    Utils::openFile( dataFile, DATA_OUTPUT_PATH + getStateBarsBaseFilename() + ".dat", fstream::out );

    for (std::list<size_t>::const_iterator it=boundaries.begin(); it!=boundaries.end(); it++) {

        front = *it;
        back  = *(++it);

        PowerState_t thisPowerState( hist, front, back );
        if (thisPowerState.numDataPoints > 5) {  // if numDataPoints is really low then we don't care about this powerState
            powerStates.push_back( thisPowerState );
            std::cout << powerStates.back() << std::endl;
            powerStates.back().outputStateBarsLine( dataFile );
        }
    }
    dataFile.close();

    drawHistWithStateBars( hist );
}

const string Signature::getStateBarsBaseFilename() const
{
    return deviceName + "-stateBars";
}

/**
 * @brief Fill in gaps in the powerStates so that each powerState in the list is nose-to-tail
 *
 * @deprecated not actually used.
 */
void Signature::fillGapsInPowerStates(
        const Histogram& hist,
        const bool verbose
        )
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

            if (verbose) cout << "Inserting new power state between " << prevPowerState->max << " and " << powerState->min << endl;

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


const PowerStateSequence& Signature::getPowerStateSequence()
{
    if ( powerStateSequence.empty() )
        updatePowerStateSequence();

    return powerStateSequence;
}

/**
 * @brief Finding out where each 'powerState' starts and ends.
 *
 * This must be called after 'powerStates' has been populated
 */
void Signature::updatePowerStateSequence()
{
    cout << "Getting power state sequence for " << deviceName << "..." << endl;

    // sanity check
    assert( ! powerStates.empty() );

    // Smooth data
    Array<Sample_t> RA; // array to hold rolling average
    rollingAv( &RA, PREPROCESSING_DATA_SMOOTHING );

    PowerStateSequenceItem powerStateSequenceItem;
    PowerStates_t::const_iterator currentPowerState;

    enum { WITHIN_STATE, NO_MANS_LAND } state;
    state = NO_MANS_LAND;

    // Go through raw reading finding occurrences of each power state
    for (size_t i=0; i<RA.getSize(); i++ ) {
        currentPowerState = getPowerState( RA[i] );

        switch (state) {
        case WITHIN_STATE:
            if (currentPowerState != powerStateSequenceItem.powerState) {
                        // then we've hit a powerState transition
                        // so store the details of the powerState we've just left
                        powerStateSequenceItem.endTime   = i*samplePeriod;
                        powerStateSequence.push_back( powerStateSequenceItem ); // save a copy

/*                        cout << "Power state transition. start=" << powerStateSequenceItem.startTime
                                << "\tendTime=" << powerStateSequenceItem.endTime
                                << "\tduration=" << powerStateSequenceItem.endTime - powerStateSequenceItem.startTime
                                << "\t" << *powerStateSequenceItem.powerState << endl; */

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
        powerStateSequenceItem.endTime = RA.getSize()*samplePeriod;
        powerStateSequence.push_back( powerStateSequenceItem ); // save a copy
//        cout << "Power state transition. start=" << powerStateSequenceItem.startTime
//                << "\tendTime=" << powerStateSequenceItem.endTime
//                << "\tduration=" << powerStateSequenceItem.endTime - powerStateSequenceItem.startTime
//                << "\t" << *powerStateSequenceItem.powerState << endl;
    }

    cout << "...done getting power state sequence for " << deviceName << "." << endl;
    powerStateSequence.plotGraph();

    /** @todo Detect repeats */
}


/**
 * @return pointer to a 'powerState' within 'powerStates' corresponding to 'sample'.
 *         Returns 'powerStates.end()' if 'sample' does not correspond to any 'powerState'.
 */
PowerStates_t::const_iterator Signature::getPowerState( const Sample_t sample ) const
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
    return powerStates.end();
}


/**
 * @brief Takes the gradient of this Signature,
 *        then merges gradient values with the same sign,
 *        then removes transient spikes, then sorts by value.
 *
 * @return a list of gradient Spikes, sorted in descending order of absolute 'value'.
 *
 */
const list<Signature::Spike> Signature::getDeltaSpikes(
        const size_t LOOK_AHEAD  /**< check there isn't an opposite-sign
                                      spike within this number of samples. */
    ) const
{
    // CONSTANTS
    const double LOOK_AHEAD_TOLLERANCE = 0.2; // what qualifies as a "similar sized" spike? 0==exactly equal.  0.1==within 10% of first spike's value.

    // LOCAL VARIABLES
    list<Spike>::iterator  spike, lookAhead;
    list<Spike> mergedSpikes = getMergedSpikes();
    bool found;

    /* Remove fleetingly transient spikes */
    spike = mergedSpikes.begin();
    while ( spike != mergedSpikes.end() ) {
        // Check spike isn't fleetingly transient
        lookAhead = spike;
        advance( lookAhead, 1 );
        /* Look ahead to see if there's a spike of opposite sign and similar magnitude
         * within LOOK_AHEAD samples.     */
        found = false;
        while ( lookAhead!=mergedSpikes.end() && (lookAhead->index - spike->index)<LOOK_AHEAD ) {
            if ( Utils::roughlyEqual( spike->delta, -lookAhead->delta, LOOK_AHEAD_TOLLERANCE ) ) {
                mergedSpikes.erase( lookAhead++ );
                mergedSpikes.erase( spike++ );
                found = true;
                break;
            }
            lookAhead++;
        }
        if ( ! found )
            spike++;
    }

    // Sort by spike.value
    mergedSpikes.sort( Spike::compareAbsValueDesc );

    return mergedSpikes;
}

/**
 * @brief Merge consecutive spikes of the same sign and
 *        then remove any spikes under 10 Watts.
 *
 * @return a list of merged spikes
 *
 * NOTE: returning a @c std::list should not be costly because
 * both GCC and MSVC implement Return Value Optimisation,
 * even when optimisation is turned off.
 * http://stackoverflow.com/questions/1092561/is-returning-a-stdlist-costly
 */
const list<Signature::Spike> Signature::getMergedSpikes() const
{
    // LOCAL VARIABLES
    list<Spike> mergedSpikes;
    Spike spikeToStore;
    double currentDelta, lastDelta;

    spikeToStore.n = spikeToStore.index = 0;
    spikeToStore.delta = lastDelta = getDelta((size_t)0);

    for (size_t i=1; i<(size-1); i++) {

        currentDelta = getDelta(i);

        /* Check to see if this gradient is a continuation of
         * a larger spike.  i.e. does this gradient have the
         * same sign as the last gradient. */
        spikeToStore.n++;
        if ( ((lastDelta > 0.0) && (currentDelta > 0.0)) || // check both this and last spike have same sign and are non-zero
                ((lastDelta < 0.0) && (currentDelta < 0.0))    ) {
            // the current spike and the last spike are immediately
            // adjacent, and both spikes are the same sign.  So these
            // spikes need to be merged and not stored in mergedGradient yet.
            spikeToStore.delta += currentDelta;
        }
        else if ((lastDelta == 0.0) && (currentDelta != 0.0)) { // are we starting a new spike after having been at zero?
            spikeToStore.index = i;
            spikeToStore.n = 0;
            spikeToStore.delta = currentDelta; // reset mergedGradient
        }
        else if ((lastDelta == 0.0) && (currentDelta == 0.0)) { // we're in no man's land
            // do nothing
        }
        else {
            // store
            mergedSpikes.push_back( spikeToStore );

            // reset
            spikeToStore.delta = currentDelta;
            spikeToStore.n = 0;
            spikeToStore.index = i;
        }

        lastDelta = currentDelta;
    }

    /* remove any spikes under 10 Watts
     * (because these are very unlikely to be found in the
     * aggregate data)   */
    list<Spike>::iterator spike = mergedSpikes.begin();
    while (spike != mergedSpikes.end()) {
        if (fabs(spike->delta) < 10)
            mergedSpikes.erase( spike++ );
        else
            spike++;
    }


    return mergedSpikes;
}


/**
 * @brief Convert from the Signature data to a different sample period.
 *
 * If the original sample period is 1 second and newPeriod = 6, then the returned
 * array will have 1/6th the number of entries and each entry in the downsampled
 * array will be an average of 6 samples from the original array.
 *
 * @deprecated Probably better to use rollingAv().
 *
 * @todo this should be in Array shouldn't it?
 *
 * @return The parameter @c output is the returned, down sampled array.
 */
void Signature::downSample(
        Array<Sample_t> * output,
        const size_t newPeriod,
        const bool verbose
        ) const
{
    size_t inner, inputIndex=0, outputIndex, outerLimit;
    Sample_t accumulator;

    const size_t newSize = (int)ceil(size / ( (double)newPeriod / (double)samplePeriod ));
    output->setSize( newSize );


    const size_t mod = newPeriod % samplePeriod;
    if (mod==0) {
        // newPeriod % samplePeriod == 0; so then just take an average of each step
        const size_t stepSize = newPeriod/samplePeriod;
        const size_t delta = (newSize*newPeriod) - (size*samplePeriod);

        if (delta==0) {
            outerLimit = newSize;
        } else {
            outerLimit = newSize-1;
        }

        if (verbose) {
            cout  << "old size=" << size << ", old period=" << samplePeriod
                  << ", new size=" << newSize << ", newPeriod=" << newPeriod
                  << ", stepSize=" << stepSize << ", mod=" << mod << ", delta=" << delta << ",outerLimit=" << outerLimit << endl;
        }

        // Do the vast majority of the resampling
        for (outputIndex=0; outputIndex<outerLimit; outputIndex++) {
            accumulator=0;
            for (inner=0; inner<stepSize; inner++) {
                inputIndex = inner+(outputIndex*stepSize);
                accumulator += data[ inputIndex ];
            }
            (*output)[ outputIndex ] = accumulator/stepSize;
        }

        // Do the remainder (if there is any)
        if (delta != 0) {
            accumulator=0;
            size_t i;
            for (i=inputIndex; i<size; i++) {
                accumulator += data[i];
            }
            (*output)[ newSize-1 ] = accumulator/(delta-1);
        }

    } else {
        // newPeriod%samplePeriod!=0 so interpolate
        cerr << "resample() doesn't yet deal with fractional sample rate conversions" << endl;
    }
}

const size_t Signature::getSamplePeriod() const
{
    return samplePeriod;
}

/**
 * @brief Determine how many Joules of energy have been consumed.
 */
const double Signature::getEnergyConsumption() const
{
    double accumulator = 0;
    for (size_t i=0; i<size; i++) {
        accumulator += data[i];
    }

    return accumulator * samplePeriod;
}

const size_t Signature::getID() const
{
    return sigID;
}
