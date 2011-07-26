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

private:
    /************************
     *  Member functions    *
     ************************/
    void updatePowerStates();
    void updatePowerStateSequence();

    struct currentCostReading; // forward declaration
    void loadCurrentCostData(std::fstream& fs, Array<currentCostReading> * aggData);

    const double LMDiff(
            const size_t agOffset,
            const Array<currentCostReading>& aggData, // aggregate data array
            const Array<Sample_t>& sigArray,
            const size_t aggDataSamplePeriod);

    /************************
     *  Member variables    *
     ************************/
    std::string name;
    PowerStates_t powerStates;
    std::vector<Signature*> signatures;
    PowerStateSequence powerStateSequence;

    /**
     * @todo currentCostReading shouldn't be in Device. It should probably be
     * Array -> AggregateReading -> CurrentCost
     */
    struct currentCostReading {
        size_t timestamp;
        size_t reading;

        friend std::ostream& operator<<(std::ostream& o, const currentCostReading& ccr)
        {
            o << ccr.timestamp << "\t" << ccr.reading << std::endl;
            return o;
        }

    };


};

#endif /* DEVICE_H_ */
