/*
 * Main.cpp
 *
 * Main file for disaggregate program.
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#include <boost/program_options.hpp>
namespace po = boost::program_options;


#include "Array.h"
#include "Signature.h"
#include "Statistic.h"
#include "Device.h"
#include "PowerStateGraph.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

using namespace std;


void powerStateGraphTest()
{
    AggregateData aggData;
    aggData.loadCurrentCostData( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv" );
//    aggData.loadCurrentCostData( "data/input/watts_up/washer2-timestamped.csv" );
//    aggData.loadCurrentCostData( "data/input/current_cost/test.csv" );

//    for (size_t i=0; i<aggData.getSize(); i++) {
//        cout << aggData.aggDelta(i) << endl;
//    }

    PowerStateGraph psg;
//    Signature sig( "data/input/watts_up/kettle.csv", 1, "kettle" );
//    Signature sig2( "data/input/watts_up/kettle2.csv", 1, "kettle2" );
//    Signature sig( "data/input/watts_up/toaster.csv", 1, "toaster" );
//    Signature sig( "data/input/watts_up/tumble.csv", 1, "tumble", 1, 1, 6600 );

//    Signature sig( "data/input/watts_up/test.csv", 1, "test" );

    Signature sig2( "data/input/watts_up/washer2.csv", 1, "washer2", 1,1, 2000 );

//    Signature sig( "data/input/watts_up/washer.csv", 1, "washer", 1, 1, 2530 );

    psg.update( sig2 );
//    psg.update( sig );


    cout << endl
         << "Power State Graph vertices:" << endl
         << psg << std::endl;

    // output power state graph to file
    fstream fs;
    const string psgFilename = DATA_OUTPUT_PATH + "powerStateGraph.gv";
    cout << "Outputting power state graph to " << psgFilename << endl;
    Utils::openFile(fs, psgFilename, fstream::out);
    psg.writeGraphViz( fs );
    fs.close();

    psg.getStartTimes( aggData );

}

void printHelp(const po::options_description& desc)
{
    cout << desc << endl
         << "Example usage:" << endl
         << "   ./disaggregate dataCroppedToKettleToasterWasherTumble -s kettle -s kettle2" << endl << endl;
}

int main(int argc, char * argv[])
{
    string aggData;

    cout << endl << "SMART METER DISAGGREGATION TOOL" << endl;

    // Declare the supported options
    po::positional_options_description p;
    p.add("aggdata", -1);

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h",
                    "produce help message\n")
            ("aggdata,a",
                    po::value<string>(&aggData)->default_value("dataCroppedToKettleToasterWasherTumble"),
                    "aggregate data file (without path or suffix).\n")
            ("signatures,s",
                    po::value< vector<string> >(),
                    "device signature file(s) (without path or suffix).\n");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options( desc ).positional(p).run() , vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << endl << desc << endl << endl;
        exit(1);
    }

    if (vm.count("signatures")) {
        cout << "Signatures set to:" << endl;
        for (vector<string>::const_iterator s=vm["signatures"].as< vector<string> >().begin();
                s!=vm["signatures"].as< vector<string> >().end(); s++) {
            cout << *s << endl;
        }
    } else {
        cout << endl << "Signature file(s) must be specified using the --signatures option." << endl << endl;
        printHelp( desc );
        exit(1);
    }

    if (vm.count("aggdata")) {
        cout << "Aggregate data set to " << vm["aggdata"].as< string >() << endl;
    }

    /*
    Signature sig( "washer.csv", 1 );
    Histogram ha;
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

//      Device washer("Washer2");
//    washer.getReadingFromCSV( "data/input/watts_up/washer2.csv", 1, 1, 1 );
//      washer.getReadingFromCSV( "data/input/watts_up/washer.csv", 1, 50, 2200 );
//    washer.getReadingFromCSV( "data/input/watts_up/washer2.csv", 1, 1, 1 );
//    washer.findAlignment( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv", 6);

//    AggregateData aggData;
//    aggData.loadCurrentCostData( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv" );
//      washer.getStartTimes( aggData );


    powerStateGraphTest();

//    washer.findAlignment( "data/input/watts_up/washer2.csv", 1);

/*    Device tumble("tumble");
    tumble.getReadingFromCSV( "data/input/watts_up/tumble.csv", 1, 1, 6500 );
    tumble.findAlignment( "data/input/current_cost/dataCroppedToKettleToasterWasherTumble.csv", 6);
*/
    /*
    Array<int> raTest(10, pop);
    Array<Sample_t> raArray;
    raTest.rollingAv(&raArray,7);
    std::cout << raArray << std::endl;*/

    cout << endl << "Finished." << endl << endl;

    return 0;
}
