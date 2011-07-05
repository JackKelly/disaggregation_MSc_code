/*
 * Disaggregate.cpp
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#include "Disaggregate.h"
#include "Array.h"
#include <iostream>
#include <glog/logging.h>

int main(int argc, char * argv[])
{
    google::InitGoogleLogging(argv[0]);
    Array<int> a(10000);

    std::cout << "Test complete." << std::endl;

    google::ShutdownGoogleLogging();
    return 0;
}
