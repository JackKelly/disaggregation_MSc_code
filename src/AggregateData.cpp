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
    std::cout << "Loading CurrentCost data " << filename << " ...";

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
    LOG(INFO) << "Entered " << count << " ints into data array.";

    std::cout << "... done." << std::endl;
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
 * @deprecated a relic from an unfinished previous attempt to disaggregate
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

list<AggregateData::FoundSpike> AggregateData::findSpike(
        const Statistic<Sample_t>& spikeStats,
        size_t startTime,
        size_t endTime,
        double stdevMultiplier /**< Higher = more permissive */
        ) const
{
    cout << "...findSpike looking for " << spikeStats << " from startTime=" << startTime << " endTime=" << endTime << endl;

    size_t i;
    list<AggregateData::FoundSpike> foundSpikes;

    // lots of range checking and default parameter setting
    if (endTime == 0) { // default parameter used
        endTime = data[size-1].timestamp;
    }
    else if (endTime > data[size-1].timestamp) {
//        LOG(WARNING) << "(endTime > data[size-1].timestamp)... setting endTime = data[size-1].timestamp; ";
        endTime = data[size-1].timestamp;
    }

    if (startTime == 0) { // default parameter used
        startTime = data[0].timestamp;
        i = 0;
    }
    else if (startTime < data[0].timestamp) {
//        LOG(WARNING) << "(startTime < data[0].timestamp)... setting startTime = data[0].timestamp;";
        startTime = data[0].timestamp;
        i = 0;
    }
    else {
        i = findTime(startTime);
    }

    assert( endTime > startTime );

    // (if the statistic was created from a single value then stdev will
    //  already have been "faked" by the single-value constructor
    //  in Statistic.h )
    const double stdev = spikeStats.stdev;

    boost::math::normal dist(spikeStats.mean, stdev);
    // see http://live.boost.org/doc/libs/1_42_0/libs/math/doc/sf_and_dist/html/math_toolkit/dist/dist_ref/dists/normal_dist.html

    size_t time = startTime;
    while (time < endTime && i < size) {

        // Are spikeStats.mean and aggDelta(i) within stdev*stdevMultiplier of eachother?
        if ( Utils::within(aggDelta(i), spikeStats.mean, stdev*stdevMultiplier) ) {

            foundSpikes.push_back(
                    FoundSpike(
                            time,
                            aggDelta(i),
                            boost::math::pdf(dist, aggDelta(i))
                            ) );
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
 *
 * @return the index at which @time is located.
 */
const size_t AggregateData::findTime(
        const size_t time
    ) const
{
    if (time > data[size-1].timestamp || time < data[0].timestamp) {
        LOG(FATAL) << "Timestamp is out of range.  Fatal error.";
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
