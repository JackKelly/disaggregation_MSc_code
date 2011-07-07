/*
 * Disaggregate.cpp
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#include "Disaggregate.h"
#include "Array.h"
#include "Signature.h"
#include <iostream>
#include <glog/logging.h>
#include <fstream>

int main(int argc, char * argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();

    Signature sig("washer.csv", 1);

    SigArray a;
    sig.downSample(a, 100);

    std::fstream fs;
    fs.open("output100.csv", std::ifstream::out);
    if (!fs.good()) {
        LOG(FATAL) << "Can't open output.csv";
    }
    fs << a;
    fs.close();

    LOG(INFO) << "Shutting down...";
    google::ShutdownGoogleLogging();
    return 0;
}
