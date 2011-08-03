/*
 * AggregateData.h
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#ifndef AGGREGATEDATA_H_
#define AGGREGATEDATA_H_

#include "Array.h"
#include <string>
#include <iostream>
#include <fstream>

struct AggregateSample {
    size_t timestamp;
    size_t reading;

    friend std::ostream& operator<<(std::ostream& o, const AggregateSample& as)
    {
        o << as.timestamp << "\t" << as.reading << std::endl;
        return o;
    }

};

class AggregateData : public Array<AggregateSample>
{
public:

    void loadCurrentCostData(
            const std::string& filename
            );

    const size_t secondsSinceFirstSample( const size_t i ) const;

    const size_t aggDelta( const size_t i ) const;

    const size_t findNear(
            const size_t candidateIndex,
            const size_t expectedDistance,
            const size_t delta
            ) const;

private:

    size_t samplePeriod; /**< in seconds. */
};

#endif /* AGGREGATEDATA_H_ */
