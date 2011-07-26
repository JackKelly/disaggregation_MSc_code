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
    Array<Histogram_t> ha;
    sig.getRawReading().histogram( &ha );
    ha.dumpToFile( "data/input/processed/histogram.csv" );

    Array<Sample_t> a;
    sig.downSample( &a, 100 );

    Array<Sample_t> raHist;
    ha.rollingAv(&raHist,39);
    raHist.dumpToFile( "data/input/processed/raHistogram39.csv" );

    int pop[10] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> stdevTest(10, pop);
    Statistic<int> stat(stdevTest);

    std::cout << stat << std::endl;
*/

/*    Device toaster("Toaster");
    toaster.getReadingFromCSV( "data/input/watts_up/toaster.csv", 1 ,0 ,0 );
    toaster.findAlignment( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv", 6);
*/

/*    Device kettle("Kettle");
    kettle.getReadingFromCSV( "data/input/watts_up/kettle.csv", 1 ,1 ,1);
    kettle.findAlignment( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv", 6);
*/

      Device washer("Washer1");
//    washer.getReadingFromCSV( "data/input/watts_up/washer2.csv", 1, 1, 8000 );
      washer.getReadingFromCSV( "data/input/watts_up/washer.csv", 1, 50, 2200 );
//    washer.getReadingFromCSV( "data/input/watts_up/washer2.csv", 1, 1, 1 );
//    washer.findAlignment( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv", 6);


//    washer.findAlignment( "data/input/watts_up/washer2.csv", 1);

/*    Device tumble("tumble");
    tumble.getReadingFromCSV( "data/input/watts_up/tumble.csv", 1, 0, 0 );
    tumble.findAlignment( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv", 6);
*/
    /*
    Array<int> raTest(10, pop);
    Array<Sample_t> raArray;
    raTest.rollingAv(&raArray,7);
    std::cout << raArray << std::endl;*/

    LOG(INFO) << "Shutting down...";
    google::ShutdownGoogleLogging();
    return 0;
}
