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
    Device();
    virtual ~Device();

    void getReadingFromCSV(const char * filename, const size_t samplePeriod);

private:
    /************************
     *  Member functions    *
     ************************/
    void updatePowerStates(const Array<Sample_t>& readings);


    /************************
     *  Member variables    *
     ************************/
    std::list<Statistic<Sample_t>*> powerStates;
    std::vector<Signature*> signatures;
    std::string name;
};

#endif /* DEVICE_H_ */
