/*
 * Array.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include "Common.h"
#include "Utils.h"
#include <iostream>
#include <glog/logging.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cassert>
#include <list>

template <class T> struct Array;  // forward declaration so the Histogram_t typedef works
typedef Array<uint32_t> Histogram_t;
typedef Array<double>   RollingAv_t;

/**
 * Very simple, light weight Array struct.
 */
template <class T>
struct Array {
    T * data;
    size_t size; // size_t is 8bytes wide on x86_64

    Array() : data(0), size(0)
    {}

    explicit Array(const size_t _size) : data(0), size(0)
    {
        setSize(_size);
    }

    /**
     * Constructor for building an Array from an existing C-array
     *
     * @param _size
     * @param _data
     */
    explicit Array(const size_t _size, const T * _data) : data(0), size(0)
    {
        setSize(_size);
        for (size_t i=0; i<size; i++) {
            data[i] = _data[i];
        }
    }

    /**
     * Copy constructor
     */
    Array(const Array<T>& other) : data(0), size(0)
    {
        setSize(other.size);

        for (size_t i=0; i<size; i++) {
            data[i] = other[i];
        }
    }

    Array<T>& operator=(const Array<T>& source)
    {
        setSize(source.size);
        for (size_t i=0; i<size; i++) {
            data[i] = source[i];
        }

        return *this;
    }

    bool operator==(const Array<T>& other)
    {
        if (size != other.size)
            return false;

        for (size_t i=0; i<size; i++) {
            if (data[i] != other[i])
                return false;
        }

        return true;
    }

    ~Array()
    {
        if ( data != 0 ) {
            delete [] data;
        }
    }

    /**
     * Mutable subscript operator.  Fast.  No range checking
     */
    T& operator[](const size_t i)
    {
        return data[i];
    }

    /**
     * Const subscript operator.  Fast.  No range checking
     */
    const T& operator[](const size_t i) const
    {
        return data[i];
    }

    void setSize(const size_t _size)
    {
        if (size == _size)
            return;

        size = _size;
        LOG(INFO) << "Setting array size = " << size;
        try {
            if ( data!=0 ) { // check if this has already been used
                LOG(INFO) << "Deleting existing Array struct to make way for a new one of size " << _size;
                delete [] data;
                data=0;
            }
            data = new T[size];
        } catch (std::bad_alloc& ba) {
            LOG(FATAL) << "Failed to allocate memory: " << ba.what();
        }
    }

    /**
     * Copy from source to this object, starting at cropFront index and
     * ignoring the last cropBack items from the source.
     *
     * @param source
     * @param cropFront
     * @param cropBack
     */
    void copyCrop(const Array<T>& source, const size_t cropFront, const size_t cropBack)
    {
        if ( source.size==0 || source.data==0 ) {
            LOG(WARNING) << "Nothing to copy.";
            return;
        }

        this->setSize( source.size - cropFront - cropBack );

        for (size_t i = 0; i<size; i++ ) {
            data[i] = source[i+cropFront];
        }
    }

    void initAllEntriesTo(const T initValue)
    {
        for (size_t i = 0; i<size; i++) {
            data[i] = initValue;
        }
    }

    /**
     * Create a histogram.
     *
     * @param hist = an empty Array<uint32_t> object
     */
    void histogram(Histogram_t * hist) const
    {
        if (hist->size == 0) {
            hist->setSize(3500); // 1 Watt resolution; max current on a 13Amp 230Volt circuit = 2990W.  Plus some headroom
        }
        hist->initAllEntriesTo(0);

        for (size_t i=0; i<size; i++) {
            (*hist)[ Utils::roundToNearestInt( data[i] ) ]++;
        }

    }

    void dumpToFile(const char* filename) const
    {
        LOG(INFO) << "Dumping Array to filename " << filename;
        std::fstream fs;
        fs.open( filename, std::ifstream::out );
        if ( ! fs.good() ) {
            LOG(FATAL) << "Can't open " << filename;
        }
        fs << *this;
        fs.close();
    }

    /**
     * Returns a rolling average of same length as the original array.
     *
     * @param ra = Initally an empty RollingAv_t.  Returned with Rolling Averages.
     * @param length = number of items to use in the average.  Must be odd.
     */
    void rollingAv(RollingAv_t * ra, const size_t length=5) const
    {
        assert( length%2 );  // length must be odd
        assert( length>1 );
        assert( length<size);

        // setup ra
        ra->setSize(this->size);

        size_t i;
        T accumulator, accumulatorDown;

        size_t middleOfLength = ((length-1)/2)+1;

        // First do the first (length-1)/2 elements of the RollingAv_t
        // and the last (length-1)/2 elements
        (*ra)[0] = data[0];
        for (i=1; i<(middleOfLength-1); i++) {
            accumulator = 0;
            accumulatorDown = 0;
            for (size_t inner=0; inner<=(i*2); inner++) {
                accumulator += data[inner];
                accumulatorDown += data[size-1-inner];
            }
            (*ra)[i] = (double)accumulator / ( (i*2) + 1 );
            (*ra)[size-1-i] = (double)accumulatorDown / ( (i*2) + 1 );
        }

        // Now do the main chunk of the array
        for (; i<=(size-middleOfLength); i++) {
            accumulator = 0;
            for (size_t inner=(i-middleOfLength+1); inner<(i+middleOfLength); inner++) {
                accumulator += data[inner];
            }
            (*ra)[i] = (double)accumulator/length;
        }

        // Do the last element of the array
        (*ra)[size-1] = data[size-1];
    }

    /**
     * Return the index of and the value of the largest element of the Array
     *
     * @param maxValue = return value
     * @param start
     * @param end
     * @return
     */
    const size_t max(T* maxValue, const size_t start=0, size_t end=0) const
    {
        if (end==0)
            end = size;

        assert( start <= end );

        if (start == end) {
            *maxValue = data[start];
            return start;
        }

        T maxSoFar=0;
        size_t indexOfMaxSoFar=start;

        for (size_t i=start; i<end; i++) {
            if (data[i] > maxSoFar) {
                maxSoFar = data[i];
                indexOfMaxSoFar = i;
            }
        }

        *maxValue = maxSoFar;
        return indexOfMaxSoFar;
    }

    /**
     * Find the max value outside of the mask.
     *
     * @param maxValue = return max value
     *
     * @param mask - a list of (mask start index, mask end index) pairs describing the mask.
     * The masks must not overlap.  The mask includes the start and end index.
     *
     * @return an index to the max value.  If there are two equal values
     * then we only return the first we come to.
     */
    const size_t max(T* maxValue, std::list<size_t>& mask) const
    {
        if ( mask.empty() ) {
            LOG(INFO) << "Mask is empty.";
            return max(maxValue, 0, size );
        }

        // check there are an even number of items in the list (start, end pairs)
        assert( ( mask.size() % 2 )==0 );

        mask.sort();

        T maxSoFar=0, maxThisLoop=0;
        size_t indexOfMaxSoFar=0, indexOfMaxThisLoop=0, start=0, end;
        std::list<size_t>::const_iterator it=mask.begin();

        while ( it != mask.end() ) {
            end = *(it++);
            indexOfMaxThisLoop = max( &maxThisLoop, start, end );
            if (maxThisLoop > maxSoFar) {
                maxSoFar = maxThisLoop;
                indexOfMaxSoFar = indexOfMaxThisLoop;
            }
            start = *(it++) + 1; // "+1" so we exclude the end of the mask
        }
        *maxValue = maxSoFar;
        return indexOfMaxSoFar;
    }

    void descendPeak(const size_t peak, size_t* start, size_t* end, size_t retries = 1)
    {
        size_t i = peak;
        size_t retriesRight = retries, retriesLeft = retries;

        // Descend right side of peak
        while ( true ) {
            i++;
            if ( i >= size ) {
                *end = size;
                break;
            }

            if ( data[i] >= data[i-1] ) {
                retriesRight--;

                if ( retriesRight == 0 ) {
                    *end = i;
                    break;
                }
            }
        }

        // Descend left side of peak
        i = peak;
        while ( true ) {
            i--;
            if ( i <= 0 ) {
                *start = 0;
                break;
            }

            if ( data[i] >= data[i+1] ) {
                retriesLeft--;

                if ( retriesLeft == 0 ) {
                    *start = i;
                    break;
                }
            }

        }
    }


    friend std::ostream& operator<<(std::ostream& o, const Array<T>& a)
    {
        for (size_t i=0; i<a.size; i++) {
            o << a[i] << std::endl;
        }
        return o;
    }
};

#endif /* ARRAY_H_ */
