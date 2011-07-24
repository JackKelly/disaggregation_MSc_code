/*
 * Statistic.h
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#ifndef STATISTIC_H_
#define STATISTIC_H_

#include <string>
#include <glog/logging.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include "Array.h"
#include "Common.h"

template <class T>
struct Statistic {
    /************************
     *  Member variables    *
     ************************/
    double mean;
    double stdev;
    T min;
    T max;
    size_t numDataPoints;
    std::string name;

    /************************
     *  Member functions    *
     ************************/

    /**
     * Constructor from histogram data
     *
     * @param data
     * @param beginning (gets included in stats)
     * @param end (excluded from stats)
     */
    Statistic(const Array<Histogram_t>& data, const size_t beginning=0, size_t end=0)
    : mean(0), stdev(0), numDataPoints(0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==0) { // default value used
            end = data.size;
        }

        assert(end >  beginning);
        assert(end <= data.size);

        min = beginning;
        max = end;

        // Find the mean, min and max
        for (size_t i=beginning; i<end; i++) {
            currentVal = data[i];

            numDataPoints += currentVal;
            accumulator   += ( currentVal * i );
        }
        mean = (double)accumulator / numDataPoints;

        // Find the population standard deviation
        accumulator = 0;
        for (size_t i=beginning; i<end; i++) {
            accumulator += pow( ( i -  mean ), 2 ) * data[i];
        }
        stdev = sqrt(accumulator / numDataPoints);
    }

    /**
     * Default constructor.
     *
     * @param data
     * @param beginning
     * @param end
     */
/*    Statistic(const Array<T>& data, const size_t beginning=0, size_t end=0)
    : mean(0), stdev(0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==0) { // default value used
            end = data.size;
        }

        assert(end >  beginning);
        assert(end <= data.size);

        size_t length = end-beginning;

        // Initialise min and max to the first value
        min = max = data[beginning];

        // Find the mean, min and max
        for (size_t i=beginning; i<end; i++) {
            currentVal = data[i];

            accumulator += currentVal;

            if ( currentVal > max )
                max = currentVal;

            if ( currentVal < min )
                min = currentVal;
        }
        mean = accumulator / length;

        // Find the population standard deviation
        accumulator = 0;
        for (size_t i=beginning; i<end; i++) {
            accumulator += pow( ( data[i] - mean ), 2 );
        }
        stdev = sqrt(accumulator / length);
    }
 */

    friend std::ostream& operator<<(std::ostream& o, const Statistic<T>& s)
    {
        o << "min=" << s.min << "\tmean=" << s.mean << "\tmax=" << s.max << "\tstdev=" << s.stdev << "\tnumDataPoints=" << s.numDataPoints;
        return o;
    }

    void xErrorBarOutputLine( std::ostream& o )
    {
        //   x              y            xlow          xhigh
        o << mean << " " << 0 << " " << min << " " << max << std::endl;
    }

};

typedef Statistic<Sample_t>     PowerState_t;
typedef std::list<PowerState_t> PowerStates_t;


#endif /* STATISTIC_H_ */
