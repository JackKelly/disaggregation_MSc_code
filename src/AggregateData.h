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

/**
 * @brief A simple struct for pairing @c timecode to @c reading
 */
struct AggregateSample {
    size_t timestamp;
    size_t reading;

    friend std::ostream& operator<<(std::ostream& o, const AggregateSample& as)
    {
        o << as.timestamp << "\t" << as.reading << std::endl;
        return o;
    }

};

/**
 * @brief This class is responsible for storing and searching aggregated data,
 * such as the data produced by the Current Cost home energy monitor.
 */
class AggregateData : public Array<AggregateSample>
{
public:

    /**
     * @brief A simple struct for representing delta spikes found by findSpike()
     */
    struct FoundSpike {
        size_t timestamp;
        Sample_t delta;
        double likelihood;

        friend std::ostream& operator<<( std::ostream& o, const FoundSpike& fs)
        {
            o << "timestamp=" << fs.timestamp << ", delta=" << fs.delta << ", pdf=" << fs.likelihood;
            return o;
        }

        FoundSpike(
                const size_t   t,
                const Sample_t d,
                const double   p
                )
        : timestamp(t), delta(d), likelihood(p)
        {}

    };

    /******************************************************************/
    /** @name General functions used by all disaggregation algorithms */
    ///@{
    void loadCurrentCostData(
            const std::string& filename
            );

    const size_t secondsSinceFirstSample( const size_t i ) const;

    const size_t getSamplePeriod() const;

    const std::string getFilename() const;
    ///@}

    const int aggDelta( const size_t i ) const;

    /****************************************************************/
    /** @name Functions used by the 'graphs and spikes' algorithms. */
    ///@{
    std::list<AggregateData::FoundSpike> findSpike(
            const Statistic<Sample_t>& spikeStats,
            size_t startTime = 0 ,
            size_t endTime = 0,
            const bool verbose = false
            ) const;

    const size_t findTime(
            const size_t time
            ) const;

    const bool readingGoesBelowPowerState(
            const size_t startTime,
            const size_t endTime,
            const Statistic<Sample_t>& powerState
            ) const;
    ///@}

    /********************************************************/
    /** @name Functions used by the 'histogram' algorithms. */
    ///@{
    const size_t findNear(
            const size_t candidateIndex,
            const size_t expectedDistance,
            const size_t delta
            ) const;
    ///@}

private:

    size_t samplePeriod; /**< @brief in seconds. */

    const size_t checkStartAndEndTimes(
            size_t * startTime,
            size_t * endTime
            ) const;

    std::string aggDataFilename; /**< @brief including path and suffix */

};

#endif /* AGGREGATEDATA_H_ */
