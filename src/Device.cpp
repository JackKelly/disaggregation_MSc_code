/*
 * Device.cpp
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#include "Device.h"
#include <list>
#include <glog/logging.h>

Device::Device() {
    // TODO Auto-generated constructor stub
    LOG(INFO) << "Creating new Device.";

}

Device::~Device() {
    for (std::vector<Signature*>::iterator it; it!=signatures.end(); it++) {
        delete *it;
    }
    for (std::list<Statistic<Sample_t>*>::iterator it; it!=powerStates.end(); it++) {
        delete *it;
    }
}

void Device::getReadingFromCSV(const char * filename, const size_t samplePeriod)
{
    Signature * sig = new Signature( filename, samplePeriod );
    signatures.push_back( sig );
//    sig->getPowerStates();

    HistogramArray_t hist;

/*    SigArray_t downSampled;
    sig->downSample( &downSampled , 4 );


    downSampled.histogram( &hist );
    hist.dumpToFile( "smoothedHist.csv" );
*/
    sig->getSigArray().histogram( &hist );

    std::list<size_t> boundaries;
    hist.findPeaks( &boundaries );

    for (std::list<size_t>::const_iterator it=boundaries.begin(); it!=boundaries.end(); it++) {
        std::cout << *it << std::endl;
    }
}
