/*
 * AggregateData.cpp
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#include "AggregateData.h"


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
 * Calculate the change between data[ @c i+1 ].reading and data[ @c i ].reading.
 *
 * Would've been nice to overload @c Array::getDelta(size_t) but
 * that returns an object of type @c T and the compiler won't let me overload
 * with a function which returns a @c size_t .
 *
 * @return \code data[i+1].reading - data[i].reading \endcode
 */
const size_t AggregateData::aggDelta(
        const size_t i
        ) const
{
    if ( i > (size-2) )
        return 0;
    else
        return data[i+1].reading - data[i].reading;
}

/**
 * Find 'delta'.  Start searching 'expectedDistance' away from 'candidateIndex'
 * @return AggregateData index where best delta match was found
 */
const size_t AggregateData::findNear(
        const size_t candidateIndex, /**< index of the aggregateData */
        const size_t expectedDistance, /**< in seconds away from candidateIndex */
        const size_t delta /**< the delta we're looking for */
) const
{

}
