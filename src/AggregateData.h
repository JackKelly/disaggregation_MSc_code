/*
 * AggregateData.h
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#ifndef AGGREGATEDATA_H_
#define AGGREGATEDATA_H_

#include "Array.h"
#include "Statistic.h"
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

    struct FoundSpike {
        size_t timestamp;
        Sample_t delta;
        double pdf;  /**< probability */

        friend std::ostream& operator<<( std::ostream& o, const FoundSpike& fs)
        {
            o << "timestamp=" << fs.timestamp << ", delta=" << fs.delta << ", pdf=" << fs.pdf;
            return o;
        }

        FoundSpike(
                const size_t   t,
                const Sample_t d,
                const double   p
                )
        : timestamp(t), delta(d), pdf(p)
        {}

    };

    void loadCurrentCostData(
            const std::string& filename
            );

    const size_t secondsSinceFirstSample( const size_t i ) const;

    const int aggDelta( const size_t i ) const;

    std::list<AggregateData::FoundSpike> findSpike(
            const Statistic<Sample_t>& spikeStats,
            size_t startTime = 0 ,
            size_t endTime = 0,
            double stdevMultiplier = 2,
            const bool verbose = true
            ) const;

    const size_t findNear(
            const size_t candidateIndex,
            const size_t expectedDistance,
            const size_t delta
            ) const;

    const size_t findTime(
            const size_t time
    ) const;

private:

    size_t samplePeriod; /**< @brief in seconds. */

};

#endif /* AGGREGATEDATA_H_ */
