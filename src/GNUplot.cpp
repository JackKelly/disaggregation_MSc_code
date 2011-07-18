/*
 * GNUplot.cpp
 *
 *  Created on: 18 Jul 2011
 *      Author: jack
 */

#include "GNUplot.h"
#include <string>
#include <stdio.h>

using namespace std;

GNUplot::GNUplot() throw( string )
{
    gnuplotpipe = popen( "gnuplot" , "w" );
    if ( gnuplotpipe ) {
        isGood = true;
    } else {
        throw ("Gnuplot not found!");
        isGood = false;
    }

}

GNUplot::~GNUplot()
{
    operator()( "exit" );
    pclose( gnuplotpipe );
}

void GNUplot::operator()(const string& command)
{
    fprintf( gnuplotpipe, "%s\n", command.c_str() );
    fflush( gnuplotpipe );
}

const bool GNUplot::good()
{
    return isGood;
}
