/*
 * Utils.cpp
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#include "Utils.h"
#include <cmath> // floor()

int Utils::roundToNearestInt( const double input )
{
    return floor( input + 0.5 );
}
