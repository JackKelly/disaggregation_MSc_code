/*
 * Utils.cpp
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#include "Utils.h"
#include "Common.h"
#include <cmath> // floor()
#include <time.h>
#include <fstream>
#include <glog/logging.h>
#include <iostream>

const int Utils::roundToNearestInt( const double input )
{
    return floor( input + 0.5 );
}

const size_t Utils::roundToNearestSizeT( const double input )
{
    if (input < 0) {
//        std::cout << "WARNING: input to roundToNearestSizeT is negative." << std::endl;
        return 0;
    }

    return floor( input + 0.5 );
}

/**
 *
 * @return statically allocated time and date formated like
 *         "Tue Feb 1 21:39:46 2001 \n\0".
 *         For more details, see p188 of The Linux Programming Interface
 */
const char * Utils::todaysDateAndTime()
{
    const time_t timep = time(NULL);
    return ctime( &timep );
}

const std::string Utils::size_t_to_s( size_t num )
{
    std::string str, digit;

    do {
        digit = ('0' + ( num%10 )); // last digit
        str = digit + str;
        num /= 10;
    } while (num != 0);

    return str;
}

void Utils::openFile(
        std::fstream& fs,
        const std::string& filename,
        const std::fstream::openmode openmode
        )
{
    fs.open( filename.c_str(), openmode );

    if ( ! fs.good() ) {
        LOG(ERROR) << "Failed to open " << filename << " for reading.";
        exit(EXIT_ERROR);
    }
    LOG(INFO) << "Successfully opened " << filename;
}

/**
 * @brief Find the number of numeric data points in file.
 *
 * @return number of data points.
 */
const size_t Utils::countDataPoints( std::fstream& fs )
{
    int count = 0;
    char line[255];

    while ( ! fs.eof() ) {
        fs.getline( line, 255 );
        if ( isdigit(line[0]) ) {
            count++;
        }
    }

    LOG(INFO) << "Found " << count << " data points in file.";

    // Return the read pointer to the beginning of the file
    fs.clear();
    fs.seekg( 0, std::ios_base::beg ); // seek to beginning of file

    return count;
}

/**
 * @return true if 'filename' exists
 */
const bool Utils::fileExists( const std::string& filename )
{
    std::ifstream ifile( filename );
    return (bool)ifile;
}

/**
 * @brief Compares 2 doubles.  Are they within (a * tolerance) of each other?
 */
const bool Utils::roughlyEqual(
        const double a,
        const double b,
        const double tolerance /**< usually a fraction between 0 and 1. */
        )
{
    return fabs(a-b) <= fabs(tolerance*a);
}

/**
 * @return the larger of the two values
 */
const double& Utils::largest(
        const double& a,
        const double& b
        )
{
    if ( a > b )
        return a;
    else
        return b;
}

const double& Utils::smallest(
        const double& a,
        const double& b
        )
{
    if ( a < b )
        return a;
    else
        return b;
}

/**
 * @return the larger of the two values
 */
const size_t& Utils::largest(
        const size_t& a,
        const size_t& b
        )
{
    if ( a > b )
        return a;
    else
        return b;
}

const size_t& Utils::smallest(
        const size_t& a,
        const size_t& b
        )
{
    if ( a < b )
        return a;
    else
        return b;
}



/**
 * @return true if @c a and @b and within @c diff of each other.
 */
const bool Utils::within(
        const double a,
        const double b,
        const double diff)
{
    if ( between(a-diff, a+diff, b) )
        return true;
    else
        return false;
}

/**
 * @return true if @c sample is between @c bound1 and @c bound2.
 */
const bool Utils::between(
        const double bound1,
        const double bound2,
        const double sample
        )
{
    double lower, upper;
    if ( bound1 < bound2 ) {
        lower = bound1;
        upper = bound2;
    } else {
        lower = bound2;
        upper = bound1;
    }

    if ( sample <= upper && sample >= lower )
        return true;
    else
        return false;
}
