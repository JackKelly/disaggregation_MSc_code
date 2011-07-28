/*
 * Utils.h
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <fstream>

namespace Utils {

int  roundToNearestInt( const double input );

const char * todaysDateAndTime();

const std::string size_t_to_s( size_t num );

void openFile(
        std::fstream& fs,
        const std::string& filename,
        const std::fstream::openmode openmode);

const size_t countDataPoints( std::fstream& fs );

const bool fileExists( const std::string& filename );

const bool roughlyEqual(
        const double a,
        const double b,
        const double tolerance
        );

} /* namespace Utils */

#endif /* UTILS_H_ */
