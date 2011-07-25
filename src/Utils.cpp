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

int Utils::roundToNearestInt( const double input )
{
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
 * Find the number of numeric data points in file
 *
 * @param fs
 * @return number of data points
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


