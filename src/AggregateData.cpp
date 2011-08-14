/*
 * AggregateData.cpp
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#include "AggregateData.h"
#include <boost/math/distributions/normal.hpp>
#include <cassert>

using namespace std;

void AggregateData::loadCurrentCostData(
        const std::string& filename /**< including path and suffix. */
    )
{
    cout << "Loading CurrentCost data " << filename << "..." << endl;

    samplePeriod = 6;

    std::fstream fs;
    Utils::openFile(fs, filename, std::fstream::in);

    setSize( Utils::countDataPoints( fs ) );

    int count = 0;
    char ch;
    while ( ! fs.eof() ) {
        ch = fs.peek();
        if ( isdigit(ch) ) {
            fs >> data[ count ].timestamp;
            fs >> data[ count ].reading;
            count++;
        }
        fs.ignore( 255, '\n' );  // skip to next line
    }

    cout << "... done loading Current Cost data." << std::endl;
}

const size_t AggregateData::secondsSinceFirstSample(
        const size_t i
        ) const
{
    return data[i].timestamp - data[0].timestamp;
}

/**
 * @brief Calculate the change between data[ @c i+1 ].reading and data[ @c i ].reading.
 *
 * Would've been nice to overload @c Array::getDelta(size_t) but
 * that returns an object of type @c T and the compiler won't let me overload
 * with a function which returns a @c size_t .
 *
 * @return \code data[i+1].reading - data[i].reading \endcode
 */
const int AggregateData::aggDelta(
        const size_t i
        ) const
{
    if ( i > (size-2) )
        return 0;
    else
        return (int)data[i+1].reading - (int)data[i].reading;
}

/**
 * @brief Find 'delta'.  Start searching 'expectedDistance' away from 'candidateIndex'. DOESN'T WORK YET.
 *
 * @return AggregateData index where best delta match was found
 *
 * @deprecated a relic from an abandoned attempt to disaggregate
 *
 * @todo unfinished (obviously)
 */
const size_t AggregateData::findNear(
        const size_t candidateIndex, /**< index into aggregateData */
        const size_t expectedDistance, /**< in seconds away from candidateIndex */
        const size_t delta /**< the delta we're looking for */
) const
{
    return 0;
}

const size_t AggregateData::checkStartAndEndTimes(
        size_t * startTime,
        size_t * endTime
        ) const
{
    size_t i=0;
    // lots of range checking and default parameter setting
    if (*endTime == 0) { // default parameter used
        *endTime = data[size-1].timestamp;
    }
    else if (*endTime > data[size-1].timestamp) {
        *endTime = data[size-1].timestamp;
    }

    if (*startTime == 0) { // default parameter used
        *startTime = data[0].timestamp;
        i = 0;
    }
    else if (*startTime < data[0].timestamp) {
        *startTime = data[0].timestamp;
        i = 0;
    }
    else {
        i = findTime(*startTime);
    }

    assert( (*endTime) > (*startTime) );

    return i;
}

/**
 * @brief Run through the raw aggregate data from
 *        @c startTime to @c endTime to make sure
 *        that no agg data sample has a lower value
 *        than @c powerState.min.
 *
 * @return if no sample between @c startTime and @c endTime
 *         is lower than @c powerState.min then return false
 *         else return true.
 */
const bool AggregateData::readingGoesBelowPowerState(
        size_t startTime,
        size_t endTime,
        const Statistic<Sample_t>& powerState
        ) const
{
    size_t i = findTime( startTime );
    i++;
    size_t time = data[i].timestamp;

    while (time < endTime && i < size) {

        if ( data[i].reading < powerState.min ) {
            return true;
        }

        time = data[++i].timestamp;
    }

    return false;
}


list<AggregateData::FoundSpike> AggregateData::findSpike(
        const Statistic<Sample_t>& spikeStats,
        size_t startTime,
        size_t endTime,
        const bool verbose
        ) const
{
    if (verbose) cout << "startTime = " << startTime << " endTime = " << endTime << endl;

    size_t i = checkStartAndEndTimes( &startTime, &endTime );
    list<AggregateData::FoundSpike> foundSpikes;

    // Make sure the stdev isn't so small that it'll not provide
    // sufficient headroom when trying to find deltas in the
    // noisy aggregate data.
    double fakedStdev = spikeStats.stdev;
    if (spikeStats.mean < 500) {
        fakedStdev = fabs(spikeStats.mean/5);
    } else if (spikeStats.mean < 1000) {
        fakedStdev = fabs(spikeStats.mean/10);
    } else {
        // spikeStats.mean > 1000
        fakedStdev = fabs(spikeStats.mean/17);
    }

    const double e = fakedStdev * 5;
    double wideLowerLimit = spikeStats.min - e;
    double wideUpperLimit = spikeStats.max + e;
    double narrowLowerLimit = spikeStats.min - (fakedStdev*1.5);
    double narrowUpperLimit = spikeStats.max + (fakedStdev*1.5);

    // make sure we're at least looking for a spike of the correct sign
    if ( ! Utils::sameSign(wideLowerLimit, spikeStats.mean) )
        wideLowerLimit = 0;
    if ( ! Utils::sameSign(wideUpperLimit, spikeStats.mean) )
        wideUpperLimit = 0;

    if (verbose) {
        cout << "Looking for spike with mean " << spikeStats.mean
             << " between vals " << wideLowerLimit
             << " to " << wideUpperLimit << " and times "
             << startTime << " - " << endTime << endl;
    }

    boost::math::normal dist(spikeStats.mean, spikeStats.stdev + numeric_limits<double>::min());
    // see http://live.boost.org/doc/libs/1_42_0/libs/math/doc/sf_and_dist/html/math_toolkit/dist/dist_ref/dists/normal_dist.html

    size_t time = startTime;
    int variations[4]; // used for trying aggDelta(i), aggDelta(i)+aggDelta(i+1), aggDelta(i)+aggDelta(i-1), aD(i-1)+aD(i)+aD(i+1)
    while (time < endTime && i < size) {

        variations[0] = aggDelta(i);

        if ( Utils::between(wideLowerLimit, wideUpperLimit, variations[0]) ) {

            variations[1] = variations[0] + aggDelta(i+1);
            variations[2] = variations[0] + aggDelta(i-1);
            variations[3] = variations[0] + aggDelta(i+1) + aggDelta(i-1);

            for (size_t var_i=0; var_i<4; var_i++) {
                if ( Utils::between(narrowLowerLimit, narrowUpperLimit, variations[var_i]) ) {

                    foundSpikes.push_back(
                            FoundSpike(
                                    time,
                                    variations[var_i],
                                    boost::math::pdf(dist, variations[var_i])
                            ) );

                    if (verbose) cout << "Spike found with delta " << variations[var_i] << " expected " << spikeStats.mean << endl;

                    break;
                }
            }
        }

        time = data[++i].timestamp;
    }

    return foundSpikes;
}

/**
 * @brief a simple and inefficient way to find a timestamp.
 * If precise time cannot be found, returns the index to the
 * sample immediately prior.
 * Terminates program if time is out of range (as this is always fatal).
 * Should probably be re-written as a hash-function.
 *
 * @return the index at which @time is located.
 */
const size_t AggregateData::findTime(
        const size_t time
    ) const
{
    if (time > data[size-1].timestamp || time < data[0].timestamp) {
        Utils::fatalError("Timestamp is out of range.  Fatal error.");
    }

    if (data[size-1].timestamp == time)
        return size-1;

    size_t i = 0;
    for (; i<size-2; i++) {
        if ( Utils::between(data[i].timestamp, data[i+1].timestamp-1, time))
            return i;
    }

    // should never get here
    return -1;
}
