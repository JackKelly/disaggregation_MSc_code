/*
 * Device.cpp
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#include "Device.h"
#include <glog/logging.h>

Device::Device() {
    // TODO Auto-generated constructor stub
    LOG(INFO) << "Creating new Device.";

}

Device::~Device() {
    for (std::vector<Signature*>::iterator it; it!=signatures.end(); it++) {
        delete *it;
    }
    for (std::list<Statistic<SigArrayDataType>*>::iterator it; it!=powerStates.end(); it++) {
        delete *it;
    }
}

void Device::getReadingFromCSV(const char * filename, const int samplePeriod)
{
    Signature * sig = new Signature( filename, samplePeriod );
    signatures.push_back( sig );
}
