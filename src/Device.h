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

class Device {
public:
    Device(const std::string _name);
    virtual ~Device();

    void getReadingFromCSV(const char * filename, const size_t samplePeriod, const size_t cropFront, const size_t cropBack);

    void dumpPowerStateSequenceToFile();

    std::list<size_t> findAlignment( const char * aggregateDataFilename, const size_t aggDataSamplePeriod );

private:
    /************************
     *  Member functions    *
     ************************/
    void updatePowerStates( const size_t rollingAvLength );

    void updatePowerStateSequence( const size_t rollingAvLength );

    PowerStates_t::const_iterator getPowerState( const Sample_t sample );

    void loadCurrentCostData( Array<size_t> * aggregateData, std::fstream& aggregateDataFile );

    const double LMS(
            const size_t agOffset,
            const Array<size_t>& aggData, // aggregate data array
            const SigArray_t& sigArray,
            const size_t aggDataSamplePeriod);

    /************************
     *  Member variables    *
     ************************/
    std::string name;
    PowerStates_t powerStates;
    std::vector<Signature*> signatures;

    struct PowerStateSequenceItem {
        PowerStates_t::const_iterator powerState; /*!< TODO: Does the same iterator point to the same element, no matter if an item is subsequently entered before the item?  */
        size_t startTime;          /*!< in seconds  */
        size_t endTime;          /*!< in seconds  */
    };

    std::list<PowerStateSequenceItem> powerStateSequence;

};

#endif /* DEVICE_H_ */
