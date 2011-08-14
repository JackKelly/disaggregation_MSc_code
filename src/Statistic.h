/*
 * Statistic.h
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#ifndef STATISTIC_H_
#define STATISTIC_H_

#include <string>
#include <cassert>
#include <cmath>
#include <iostream>
#include <boost/math/distributions/students_t.hpp>
#include <limits> // for std::numeric_limits<std::size_t>::max()
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

    std::list<T> dataStore; /**< @brief store every data point we've ever seen so we can
                                   re-calculate an accurate stdev on every update.
                                   @details Memory-hungry but accurate. DOES NOT WORK with
                                   histogram data. */

    /************************
     *  Member functions    *
     ************************/

    /**
     * @brief Default constructor
     */
    Statistic()
    : mean(0), stdev(0), min(0), max(0), numDataPoints(0)
    {}

    /**
     * @brief Constructor from a single value.  'Fakes' a standard deviation.
     */
    Statistic(
            const T value
            )
    : mean(value), stdev(0), min(value), max(value), numDataPoints(1)
    {
        dataStore.push_back(value);
    }


    /**
     * @brief Constructor from histogram data.
     */
    Statistic(
            const Histogram& data, /**< data */
            const size_t beginning=0,  /**< beginning (gets included in stats) */
            size_t end=std::numeric_limits<std::size_t>::max() /**< end (excluded from stats) */
            )
    : mean(0), stdev(0), numDataPoints(0)
    {
        register T accumulator = 0;
        register T currentVal;

        if (end==std::numeric_limits<std::size_t>::max()) { // default value used
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
        double stdevAccumulator = 0;
        for (size_t i=beginning; i<end; i++) {
            stdevAccumulator += pow( ( i -  mean ), 2 ) * data[i];
        }
        stdev = sqrt(stdevAccumulator / (numDataPoints-1));
    }

    /**
     * @brief Constructor from array data (NOT histograms)
     */
    Statistic(
            const Array<T>& data,
            const size_t beginning=0,
            size_t end=std::numeric_limits<std::size_t>::max()
            )
    : mean(0), stdev(0)
    {
        // deal with the case where beginning and end or equal or within 1 of each other
        if (beginning==end || beginning==(end-1)) {
            min = max = mean = data[beginning];
            numDataPoints = 1;
            dataStore.push_back( data[beginning] );
            return;
        }

        register T accumulator = 0;
        register T currentVal;

        if (end==std::numeric_limits<std::size_t>::max()) { // if default value used
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

            dataStore.push_back(currentVal);

            accumulator += currentVal;

            if ( currentVal > max )
                max = currentVal;

            if ( currentVal < min )
                min = currentVal;
        }
        mean = (double)accumulator / numDataPoints;

        // Find the sample standard deviation
        stdev = calcStdev();
    }

    /**
     * @brief Update an existing Statistic with new data points.
     *
     */
    void update(
            const Array<T>& data,
            const size_t beginning=0,
            size_t end=std::numeric_limits<std::size_t>::max(),
            const bool checkForOutliers = false
            )
    {

        // deal with the case where beginning and end or equal or within 1 of each other
        if (beginning==end || beginning==(end-1)) {
            update( data[beginning] );
            return;
        }


        register T accumulator = 0;
        register T currentVal;

        if (end==std::numeric_limits<std::size_t>::max()) { // if default value used
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

            //check for outliers
            if (checkForOutliers) {
                if (stdev != 0) {
                    stdev = calcStdev();
                    if (currentVal > (mean + (stdev*5)) ||
                            currentVal < (mean - (stdev*5))   ) {
                        numDataPoints--;
                        numNewDataPoints--;
                        continue;
                    }
                }
            }

            dataStore.push_back(currentVal);

            accumulator += currentVal;

            if ( currentVal > max )
                max = currentVal;

            if ( currentVal < min )
                min = currentVal;
        }

        if (numNewDataPoints > 0){
            double meanOfNewData = (double)accumulator / numNewDataPoints;
            mean = (mean * ((double)numExistingDataPoints/numDataPoints)) + (meanOfNewData * ((double)numNewDataPoints/numDataPoints) );
            /** Update the sample standard deviation with the new data. */
            stdev = calcStdev();

        }
    }


    /**
     * @brief Update an existing Statistic with data from an existing statistic.
     */
    void update(
            const Statistic<T>& otherStat
            )
    {
        Array<T> otherStatArray( otherStat.dataStore.size() );

        size_t count = 0;
        typename std::list<T>::const_iterator it;
        for (it=otherStat.dataStore.begin(); it!=otherStat.dataStore.end(); it++) {
            otherStatArray[count++] = *it;
        }

        update( otherStatArray ); // re-use the existing update() function which expects an array
    }


    const double calcStdev() const
    {
        if (numDataPoints > 1) {
            double stdevAccumulator = 0;
            for (typename std::list<T>::const_iterator i=dataStore.begin(); i!=dataStore.end(); i++) {
                stdevAccumulator += pow( ( *i - mean ), 2 );
            }
            return sqrt(stdevAccumulator / (numDataPoints-1));
        } else
            return 0;
    }


    /**
     * @brief Update an existing Statistic with a single new data point.
     *
     */
    void update(const T datum)
    {
        size_t numExistingDataPoints = numDataPoints;
        numDataPoints = numExistingDataPoints + 1;
        double prevMean = mean;

        if (numExistingDataPoints == 0) {
            max = min = datum;
        } else {
            if ( datum > max )
                max = datum;

            if ( datum < min )
                min = datum;
        }

        mean = (prevMean * ((double)numExistingDataPoints/numDataPoints))
                + (datum * ((double)1.0/numDataPoints) );

        /** Update the sample standard deviation with the new data. */
        dataStore.push_back(datum);
        stdev = calcStdev();
    }

    /**
     * @brief Uses a two-sided Student T-Test to determine
     *        if @c other.mean is 'similar' to the calling object's mean.
     *
     * @return true if @c other is 'similar' to calling object.
     */
    const bool similar(
            const Statistic<T> other,
            const double alpha=0.05        /**< significance level */
            ) const
    {
        return tTest(other) > (alpha/2);
    }

    /**
     * @brief A two-sided Student T-Test.
     *
     * Uses a Chi-Squared test for equal variances first.
     *
     * Code adapted from
     * <a href="http://www.boost.org/doc/libs/1_43_0/libs/math/doc/sf_and_dist/html/math_toolkit/dist/stat_tut/weg/st_eg/two_sample_students_t.html">boost tutorial on Comparing the means of two samples with the Students-t test</a>
     *
     * @return complement of probability (the probability
     *         that the difference is due to chance).  i.e. the
     *         higher this value, the more likely the two distributions
     *         have similar means.
     */
    const double tTest(
                const Statistic<T> other
                ) const
        {
        if (mean == other.mean)
            return true;

        // deal with case where one or both Statistic arrays only has a single data point
        if (numDataPoints == 1  ||  other.numDataPoints == 1)
            return (mean == other.mean);

        using namespace boost::math;

        // Degrees of freedom (using Chi-Squared test):
        double v = stdev * stdev / numDataPoints + other.stdev * other.stdev / other.numDataPoints;
        v *= v;
        double t1 = stdev * stdev / numDataPoints;
        t1 *= t1;
        t1 /=  (numDataPoints - 1);
        double t2 = other.stdev * other.stdev / other.numDataPoints;
        t2 *= t2;
        t2 /= (other.numDataPoints - 1);
        v /= (t1 + t2);

        // t-statistic:
        double t_stat = (mean - other.mean) / sqrt(stdev * stdev / numDataPoints + other.stdev * other.stdev / other.numDataPoints);

        // define our distribution object
        students_t dist( v );

        // calculate complement of probability (the probability that the difference is due to chance)
        double q = cdf(complement(dist, fabs(t_stat)));

        return q;
    }

    /**
     * @brief Returns stdev if stdev > mean/10, else returns mean/10
     */
    const double nonZeroStdev() const {
        if (stdev < mean/10)
            return mean/10;
        else
            return stdev;
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
