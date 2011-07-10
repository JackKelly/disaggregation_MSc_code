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

template <class T>
struct Statistic {
    /************************
     *  Member variables    *
     ************************/
    T mean;
    T stdev;
    T min;
    T max;
    std::string name;

    /************************
     *  Member functions    *
     ************************/
    Statistic(const Array<T>& data, const int beginning=0, int end=0)
    : mean(0), stdev(0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==0) { // default value used
            end = data.size;
        }

        assert(end >  beginning);
        assert(end <= data.size);

        int length = end-beginning;

        // Initialise min and max to the first value
        min = max = data[beginning];

        // Find the mean, min and max
        for (int i=beginning; i<end; i++) {
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
        for (int i=beginning; i<end; i++) {
            accumulator += pow( ( data[i] - mean ), 2 );
        }
        stdev = sqrt(accumulator / length);
    }

    friend std::ostream& operator<<(std::ostream& o, const Statistic<T>& s)
    {
        o << "mean=" << s.mean << ", min=" << s.min << ", max=" << s.max << ", stdev=" << s.stdev;
        return o;
    }

};

#endif /* STATISTIC_H_ */
