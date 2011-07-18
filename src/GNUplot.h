/*
 * GNUplot.h
 *
 *  Created on: 18 Jul 2011
 *      Author: jack
 */

#ifndef GNUPLOT_H_
#define GNUPLOT_H_

#include <string>

class GNUplot {
public:
    GNUplot() throw( std::string) ;
    virtual ~GNUplot();

    void operator() (const std::string& command);

    const bool good();

protected:
    FILE * gnuplotpipe;
    bool isGood;
};

#endif /* GNUPLOT_H_ */
