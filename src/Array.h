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

template <class T> struct Array;  // forward declaration so the HistogramArray typedef works
typedef Array<uint16_t> HistogramArray;
typedef Array<double>   RollingAvArray;

/**
 * Very simple, light weight Array struct.
 */
template <class T>
struct Array {
    T * data;
    int size;

    Array() : data(0), size(0)
    {}

    explicit Array(const int _size)
    {
        setSize(_size);
    }

    /**
     * Constructor for building an Array from an existing C-array
     *
     * @param _size
     * @param _data
     */
    explicit Array(const int _size, const T * _data)
    {
        data=0;
        setSize(_size);
        for (int i=0; i<size; i++) {
            data[i] = _data[i];
        }
    }

    /**
     * Copy constructor
     */
    Array(const Array<T>& other)
    {
        setSize(other.size);

        for (int i=0; i<size; i++) {
            data[i] = other[i];
        }
    }

    Array<T>& operator=(const Array<T>& source)
    {
        setSize(source.size);
        for (int i=0; i<size; i++) {
            data[i] = source[i];
        }
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
    T& operator[](const int i)
    {
        return data[i];
    }

    /**
     * Const subscript operator.  Fast.  No range checking
     */
    const T& operator[](const int i) const
    {
        return data[i];
    }

    void setSize(const int _size)
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

    void copyCrop(const Array<T>& source, const int cropFront, const int cropBack)
    {
        if ( source.size==0 || source.data==0 ) {
            LOG(WARNING) << "Nothing to copy.";
            return;
        }

        if ( this->size==0 || this->data==0 ) {
            this->setSize( source.size - cropFront - cropBack );
        }

        for (int i = 0; i<size; i++ ) {
            data[i] = source[i+cropFront];
        }
    }

    void initAllEntriesTo(const T initValue)
    {
        for (int i = 0; i<size; i++) {
            data[i] = initValue;
        }
    }

    /**
     * Create a histogram.
     *
     * @param hist = an empty Array<uint16_t> object
     */
    void histogram(HistogramArray * hist) const
    {
        hist->setSize(3500); // 1 Watt resolution; max current on a 13Amp 230Volt circuit = 2990W.  Plus some headroom
        hist->initAllEntriesTo(0);

        for (int i=0; i<size; i++) {
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
     * @param ra = Initally an empty RollingAvArray.  Returned with Rolling Averages.
     * @param length = number of items to use in the average.  Must be odd.
     */
    void rollingAv(RollingAvArray * ra, const int length=5) const
    {
        assert( length%2 );  // length must be odd
        assert( length>1 );
        assert( length<size);

        // setup ra
        ra->setSize(this->size);

        int i, count;
        T accumulator, accumulatorDown;

        int middleOfLength = ((length-1)/2)+1;

        // First do the first (length-1)/2 elements of the RollingAvArray
        // and the last (length-1)/2 elements
        (*ra)[0] = data[0];
        for (i=1; i<(middleOfLength-1); i++) {
            accumulator = 0;
            accumulatorDown = 0;
            for (int inner=0; inner<=(i*2); inner++) {
                accumulator += data[inner];
                accumulatorDown += data[size-1-inner];
            }
            (*ra)[i] = (double)accumulator / ( (i*2) + 1 );
            (*ra)[size-1-i] = (double)accumulatorDown / ( (i*2) + 1 );
        }

        // Now do the main chunk of the array
        for (; i<=(size-middleOfLength); i++) {
            accumulator = 0;
            count=0;
            for (int inner=(i-middleOfLength+1); inner<(i+middleOfLength); inner++) {
                accumulator += data[inner];
                count++;
            }
            (*ra)[i] = (double)accumulator/length;
        }

        // Do the last element of the array
        (*ra)[size-1] = data[size-1];
    }

    friend std::ostream& operator<<(std::ostream& o, const Array<T>& a)
    {
        for (int i=0; i<a.size; i++) {
            o << a[i] << std::endl;
        }
        return o;
    }
};

#endif /* ARRAY_H_ */
