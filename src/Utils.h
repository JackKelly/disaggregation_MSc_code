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

const int  roundToNearestInt( const double input );

const size_t roundToNearestSizeT( const double input );

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

const double& highest(
        const double& a,
        const double& b
        );

const double& lowest(
        const double& a,
        const double& b
        );

const size_t& largest(
        const size_t& a,
        const size_t& b
        );

const size_t& smallest(
        const size_t& a,
        const size_t& b
        );

const bool within(
        const double a,
        const double b,
        const double diff
        );

const bool between(
        const double bound1,
        const double bound2,
        const double sample
        );

const std::string secondsToTime(
        const size_t seconds
        );

const bool sameSign(
        const double& a,
        const double& b
        );

} /* namespace Utils */

#endif /* UTILS_H_ */
