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
#include "Common.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

using namespace std;

// FORWARD DECLARATIONS

void printVersion();
void printHelp(const po::options_description& desc);
void declareAndParseOptions( po::variables_map * vm_p, int argc, char * argv[] );
void powerStateGraphTest(const bool keep_overlapping);
void testing();


// FUNCTION DEFINITIONS

void printVersion()
{
    cout << "Version 0.1 - September 2011" << endl
         << "This is the version submitted for IC DoC MSc Individual Project." << endl
         << "Written by Jack Kelly (aka Daniel), supervised by Dr. Will Knottenbelt" << endl;
}

void printHelp(const po::options_description& desc)
{
    cout << endl
         << "Usage: ./disaggregate AGGREGATE_DATA_FILE -s SIGNATURE [OPTIONS] " << endl << endl
         << "Learns the salient features of a single device" << endl
         << " from the SIGNATURE file(s) and then searches for" << endl
         << " this device in the AGGREGATE_DATA_FILE." << endl << endl
         << desc << endl
         << "Example usage:" << endl
         << "   ./disaggregate dataCroppedToKettleToasterWasherTumble.csv -s kettle.csv -s kettle2.csv -n kettle" << endl << endl;
}

void declareAndParseOptions( po::variables_map * vm_p, int argc, char * argv[] )
{
    // first create a handy reference to make the following
    // code more readable
    po::variables_map& vm = *vm_p;

    string config_file;

    try {
        //********************************//
        // Declare the supported options  //
        //********************************//

        // Boost Program Options code adapted from
        // boost.org/doc/libs/1_42_0/doc/html/program_options/tutorial.html

        // Declare a group of options that will only be
        // allowed on command line
        po::options_description generic("Generic options");
        generic.add_options()
             ("help,h", "Produce help message\n")
             ("version,v", "Print version string\n")
             ("config,c",
                   po::value<string>(&config_file)->default_value("config/disaggregate.conf"),
                   "Configuration filename.");

        // Declare a group of options that will be
        // allowed on both the command line and in
        // config file
        po::options_description config("Configuration options");
        config.add_options()
            ("signature,s",
                  po::value< vector<string> >()->composing(),
                  "Device signature file (without path)."
                  "  Use multiple -s options to specify multiple signature files."
                  "  e.g. -s FILE1 -s FILE2"
                  "  Options specified on command line will be combined with"
                  "  options from the config file.")
            ("device-name,n",
                  po::value<string>(),
                  "The device name e.g. \"kettle\".")
            ("keep-overlapping,o",
                  "Do not remove overlapping candidates during disaggregation.");


/***********************************************
        TO BE IMPLEMENTED LATER:

        ("data-output-path",
              po::value<string>()->default_value(),
              "The path to which processed data and graph images files are output.")
        ("sig-data-path",
              po::value<string>()->default_value(),
              "The path containing raw signature data files.")
        ("agg-data-path",
              po::value<string>()->default_value("data/input/current_cost"),
              "The path containing aggregate data file(s).")
        ("gnuplot-set-terminal",
              po::value<string>(),
              "The GNUplot SET TERMINAL line.")
        ("gnuplot-output-file-extension",
              po::value<string>()->default_value("svg"),
              "The GNUplot output file extension.  e.g. \"svg\".")
************************************************/


        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::positional_options_description p;
        p.add("aggdata", -1);
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("aggdata,a",
                  po::value<string>(),
                  "Aggregate data file (without path).\n");

        //*******************//
        // Register options  //
        //*******************//
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::store(po::command_line_parser(argc, argv).
                options( cmdline_options ).positional(p).run() , vm);
        po::notify(vm);

        //******************//
        // Load config file //
        //******************//

        ifstream ifs(config_file.c_str());
        if (!ifs)
        {
            cout << "can not open config file: " << config_file << "\n";
            exit( EXIT_FAILURE );
        } else {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }

        //******************//
        // React to options //
        //******************//

        if (vm.count("help")) {
            printHelp( visible );
            exit(EXIT_SUCCESS);
        }

        if (vm.count("version")) {
            printVersion();
            exit(EXIT_SUCCESS);
        }

        if (vm.count("signature")) {
            cout << "Signatures set to:" << endl;
            for (vector<string>::const_iterator s=vm["signature"].as< vector<string> >().begin();
                    s!=vm["signature"].as< vector<string> >().end(); s++) {
                cout << SIG_DATA_PATH << *s << endl;
            }
        } else {
            cout << endl << "Signature file(s) must be specified using the -s option."
                 << endl << endl;
            printHelp( visible );
            exit(EXIT_SUCCESS);
        }

        if (vm.count("aggdata")) {
            cout << "Aggregate data set to" << endl
                 << AGG_DATA_PATH << vm["aggdata"].as< string >() << endl;
        }

    } catch(exception& e) {
        cout << e.what() << endl;
        exit( EXIT_FAILURE );
    }
}

int main(int argc, char * argv[])
{
    cout << "SMART METER DISAGGREGATION TOOL" << endl;

    // Declare are parse program config options
    po::variables_map vm;
    declareAndParseOptions( &vm, argc, argv );

    // Instantiate a device
    Device device( vm["device-name"].as< string >() );

    device.train( vm["signature"].as< vector<string> >() );

    AggregateData aggData;
    aggData.loadCurrentCostData( AGG_DATA_PATH + vm["aggdata"].as< string >() );

    device.getPowerStateGraph().getStartTimes(aggData, vm.count("keep-overlapping"));

//    powerStateGraphTest( vm.count("keep-overlapping") );

    cout << endl << "Finished." << endl << endl;

    return EXIT_SUCCESS;
}

void powerStateGraphTest(const bool keep_overlapping)
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

    Signature sig2( "data/input/watts_up/washer2.csv", 1, "washer2" );
    Signature sig( "data/input/watts_up/washer.csv", 1, "washer" );

    psg.update( sig2 );
    psg.update( sig );

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

    psg.getStartTimes( aggData, keep_overlapping );

}


void testing()
{
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
}
