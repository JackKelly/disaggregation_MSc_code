/*
 * Disaggregate.cpp
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#include "Disaggregate.h"
#include "Array.h"
#include "Signature.h"
#include "Statistic.h"
#include "Device.h"
#include <iostream>
#include <glog/logging.h>
#include <fstream>

int main(int argc, char * argv[])
{
    google::InitGoogleLogging( argv[0] );
    google::LogToStderr();

    /*
    Signature sig( "washer.csv", 1 );
    HistogramArray_t ha;
    sig.getSigArray().histogram( &ha );
    ha.dumpToFile( "histogram.csv" );

    SigArray_t a;
    sig.downSample( &a, 100 );

    RollingAv_t raHist;
    ha.rollingAv(&raHist,39);
    raHist.dumpToFile( "raHistogram39.csv" );

    int pop[10] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> stdevTest(10, pop);
    Statistic<int> stat(stdevTest);

    std::cout << stat << std::endl;
*/

    Device washer;
    washer.getReadingFromCSV( "data/watts_up/washer.csv", 1 );


    /*
    Array<int> raTest(10, pop);
    RollingAv_t raArray;
    raTest.rollingAv(&raArray,7);
    std::cout << raArray << std::endl;*/

    LOG(INFO) << "Shutting down...";
    google::ShutdownGoogleLogging();
    return 0;
}
