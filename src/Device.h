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

    void getReadingFromCSV(const char * filename, const size_t samplePeriod);

private:
    /************************
     *  Member functions    *
     ************************/
    void updatePowerStates();

    void updatePowerStateSequence();

    PowerStates_t::const_iterator getPowerState( const Sample_t sample );


    /************************
     *  Member variables    *
     ************************/
    std::string name;
    PowerStates_t powerStates;
    std::vector<Signature*> signatures;

    struct PowerStateSequenceItem {
        PowerStates_t::const_iterator powerState; /*!< TODO: Does the same iterator point to the same element, no matter if an item is subsequently entered before the item?  */
        size_t duration;          /*!< in seconds  */
    };

    std::list<PowerStateSequenceItem> powerStateSequence;

};

#endif /* DEVICE_H_ */
