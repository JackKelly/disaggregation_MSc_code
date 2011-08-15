/*
 * Device.h
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <list>
#include <vector>
#include <string>
#include "Signature.h"
#include "Statistic.h"
#include "PowerStateSequence.h"
#include "AggregateData.h"
#include "PowerStateGraph.h"

/**
 * @brief Class for representing "devices" (ie appliances like "dish washer", "lamp" etc)
 */
class Device {
public:
    Device(const std::string _name);
    virtual ~Device();

    void getReadingFromCSV(const char * filename, const size_t samplePeriod, const size_t cropFront, const size_t cropBack);

    std::list<size_t> findAlignment( const char * aggregateDataFilename, const size_t aggDataSamplePeriod );

    const std::string getName() const;

    const std::list<size_t> getStartTimes( const AggregateData& ) const;

    const std::list<Signature::Spike> getSalientSpikes() const;

    void train(
            const std::vector< std::string >& sigFiles
            );

    PowerStateGraph& getPowerStateGraph();

private:
    /************************
     *  Member functions    *
     ************************/
    void updatePowerStates();
    void updatePowerStateSequence();

    void loadCurrentCostData(std::fstream& fs, Array<AggregateSample> * aggData);

    const double LMDiff(
            const size_t agOffset,
            const Array<AggregateSample>& aggData,
            const Array<Sample_t>& sigArray,
            const size_t aggDataSamplePeriod);

    /************************
     *  Member variables    *
     ************************/
    std::string name;
    PowerStates_t powerStates;
    std::vector<Signature*> signatures;
    PowerStateSequence powerStateSequence;
    struct IndexAndDelta {
        size_t index;
        size_t delta;
    };
    PowerStateGraph powerStateGraph;

};

#endif /* DEVICE_H_ */
