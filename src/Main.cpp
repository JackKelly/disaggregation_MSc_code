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
         << "   ./disaggregate 10July.csv -s kettle.csv -s kettle2.csv -n kettle" << endl << endl;
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
                  "Do not remove overlapping candidates during disaggregation.")
            ("lms",
                  "Use Least Mean Squares approach for matching signature with aggregate data.")
            ("histogram",
                  "Use histogram approach. Full disaggregation is not implemented for this approach hence an aggregate file need not be supplied")
            ("cropfront",
                  po::value<size_t>(),
                  "The number of samples to crop off the front of the signature (only works with LMS or histogram).")
            ("cropback",
                  po::value<size_t>(),
                  "The number of samples to crop off the back of the signature (only works with LMS or histogram).");


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

        if (! vm.count("device-name")) {
            cout << endl << "A device name must be provided with the -n option,"
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
    size_t cropFront=0, cropBack=0;
    po::variables_map vm;
    declareAndParseOptions( &vm, argc, argv );

    cout.precision(3);
    cout.setf(ios::fixed);

    // Select mode of operator
    enum {LMS, GRAPHSnSPIKES, HISTOGRAM} mode;
    if (vm.count("lms")) {
        if (vm["signature"].as< vector<string> >().size() > 1) {
            Utils::fatalError( "You've specified more than 1 signature in conjunction"
                    " with the LMS mode.  LMS mode can only handle a single signature per device."
                    " Please try again with a single signature specified (you may need to"
                    " edit config/disaggregate.conf )." );
        }
        if (vm.count("cropback"))
            cropBack = vm["cropback"].as< size_t >();
        if (vm.count("cropfront"))
            cropFront = vm["cropfront"].as< size_t >();
        mode = LMS;
    } else if (vm.count("histogram")) {
        if (vm["signature"].as< vector<string> >().size() > 1) {
            Utils::fatalError( "You've specified more than 1 signature in conjunction"
                    " with the histogram mode.  histogram mode can only handle a single signature per device."
                    " Please try again with a single signature specified (you may need to"
                    " edit config/disaggregate.conf )." );
        }
        if (vm.count("cropback"))
            cropBack = vm["cropback"].as< size_t >();
        if (vm.count("cropfront"))
            cropFront = vm["cropfront"].as< size_t >();
        mode = HISTOGRAM;
    } else
        mode = GRAPHSnSPIKES;

    // Instantiate a device
    Device device( vm["device-name"].as< string >() );

    device.loadSignatures(
            vm["signature"].as< vector<string> >(),
            cropFront,
            cropBack
            );

    AggregateData aggData;
    if (mode!=HISTOGRAM) {
        if (!vm.count("aggdata")) {
            Utils::fatalError( "An aggregate data file must be supplied at the command line.");
        }
        aggData.loadCurrentCostData( AGG_DATA_PATH + vm["aggdata"].as< string >() );
    }

    switch (mode) {
    case LMS:
        cout << endl << "USING THE \"LEAST MEAN SQUARES\" APPROACH." << endl;
        device.findAlignment(aggData);
        break;
    case GRAPHSnSPIKES:
        cout << endl << "USING THE \"GRAPHS AND SPIKES\" APPROACH." << endl;
        device.trainPowerStateGraph();
        device.getPowerStateGraph().disaggregate(aggData, vm.count("keep-overlapping"));
        break;
    case HISTOGRAM:
        cout << endl << "USING THE \"HISTOGRAM\" APPROACH." << endl;
        device.getPowerStatesAndSequence();
        break;
    }

    cout << endl << "Finished." << endl << endl;

    return EXIT_SUCCESS;
}
