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
    double stdevAccumulator; /**< Used to save the accumulated <tt>pow((data[i]-mean),2)</tt>
                             during the stdev calc.
                             This is useful when we want to update the Statistic with new data points. */
    size_t numDataPoints;

    /************************
     *  Member functions    *
     ************************/

    /**
     * Constructor from histogram data.
     */
    Statistic(
            const Histogram& data, /**< data */
            const size_t beginning=0,  /**< beginning (gets included in stats) */
            size_t end=0               /**< end (excluded from stats) */
            )
    : mean(0), stdev(0), numDataPoints(0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==0) { // default value used
            end = data.getSize();
        }

        assert(end >  beginning);
        assert(end <= data.getSize());

        min = beginning;
        max = end;

        // Find the mean, min and max
        for (size_t i=beginning; i<end; i++) {
            currentVal = Utils::roundToNearestInt(data[i]*data.getSizeOfSource()); // need to multiply to un-normalise

            numDataPoints += currentVal;
            accumulator   += ( currentVal * i );
        }
        if (numDataPoints==0) {
            mean = (double)(min + max) / 2;
        } else {
            mean = (double)accumulator / numDataPoints;
        }

        // Find the sample standard deviation
        stdevAccumulator = 0;
        for (size_t i=beginning; i<end; i++) {
            stdevAccumulator += pow( ( i -  mean ), 2 ) * data[i];
        }
        stdev = sqrt(stdevAccumulator / (numDataPoints-1));
    }

    /**
     * Constructor from array data (NOT histograms)
     */
    Statistic(const Array<T>& data, const size_t beginning=0, size_t end=0)
    : mean(0), stdev(0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==0) { // if default value used
            end = data.getSize();
        }

        assert(end >  beginning);
        assert(end <= data.getSize());

        numDataPoints = end-beginning;

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
        mean = (double)accumulator / numDataPoints;

        // Find the sample standard deviation
        stdevAccumulator = 0;
        for (size_t i=beginning; i<end; i++) {
            stdevAccumulator += pow( ( data[i] - mean ), 2 );
        }
        stdev = sqrt(stdevAccumulator / (numDataPoints-1));
    }

    /**
     * Update an existing Statistic with new data points.
     *
     *  NOTE: This is a bit of a hack to get a rough
     *  new stdev and will result in an inaccurate stdev
     *  because we're not bothering to compare the old data points to the new mean.
     *  However, we don't need an especially accurate stdev so this hack will do.
     *  If we needed to do this properly then the only way I can think of
     *  is to store every value in the Statistic object
     *  (which would massively increase the size of the object)
     *  and to recalculate the stdev from scratch.
     */
    void update(const Array<T>& data, const size_t beginning=0, size_t end=0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==0) { // if default value used
            end = data.getSize();
        }

        assert(end >  beginning);
        assert(end <= data.getSize());

        size_t numNewDataPoints = end-beginning;
        size_t numExistingDataPoints = numDataPoints;
        numDataPoints = numNewDataPoints + numExistingDataPoints;

        // Find the mean, min and max
        for (size_t i=beginning; i<end; i++) {
            currentVal = data[i];

            accumulator += currentVal;

            if ( currentVal > max )
                max = currentVal;

            if ( currentVal < min )
                min = currentVal;
        }
        double meanOfNewData = (double)accumulator / numNewDataPoints;
        mean = (mean * ((double)numExistingDataPoints/numDataPoints)) + (meanOfNewData * ((double)numNewDataPoints/numDataPoints) );

        /** Update the sample standard deviation with the new data.
         *
         *  NOTE: This is a bit of a hack to get a rough
         *  new stdev and will result in an inaccurate stdev
         *  because we're not bothering to compare the old data points to the new mean.
         *  However, we don't need an especially accurate stdev so this hack will do.
         *  If we needed to do this properly then the only way I can think of
         *  is to store every value in the Statistic object
         *  (which would massively increase the size of the object)
         *  and to recalculate the stdev from scratch.
         */
        for (size_t i=beginning; i<end; i++) {
            stdevAccumulator += pow( ( data[i] - mean ), 2 );
        }
        stdev = sqrt(stdevAccumulator / (numDataPoints-1));
    }



    friend std::ostream& operator<<(std::ostream& o, const Statistic<T>& s)
    {
        o << "min=" << s.min << "\tmean=" << s.mean << "\tmax=" << s.max << "\tstdev=" << s.stdev << "\tnumDataPoints=" << s.numDataPoints;
        return o;
    }

    void outputStateBarsLine( std::ostream& o ) const
    {
        //   x              y            xlow          xhigh
        o << mean << " " << 0 << " " << min << " " << max << std::endl;
    }

    const double getMean()          const { return mean;  }
    const double getStdev()         const { return stdev; }
    const T      getMin()           const { return min;   }
    const T      getMax()           const { return max;   }
    const size_t getNumDataPoints() const { return numDataPoints; }

};


#endif /* STATISTIC_H_ */
