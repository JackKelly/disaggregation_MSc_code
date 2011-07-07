/*
 * Array.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include "Common.h"
#include <iostream>
#include <cstdlib> // for std::exit
#include <glog/logging.h>
#include <iostream>
#include <memory>

/**
 * Very simple, light weight Array struct.
 */
template <class T>
struct Array {
//    std::shared_ptr<T> data;
    T * data;
    int size;

    Array() : data(0), size(0)
    {}

    explicit Array(const int _size)
    {
        setSize(_size);
    }

    /**
     * Copy constructor
     */
    Array(const Array<T>& other)
    {
        this->size = other.size;

        for (int i=0; i<size; i++) {
            data[i] = other[i];
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
        size = _size;
        LOG(INFO) << "Creating array with size = " << size;
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

    friend std::ostream& operator<<(std::ostream& o, const Array<T> a)
    {
        for (int i=0; i<a.size; i++) {
            o << a[i] << std::endl;
        }
        return o;
    }
};

#endif /* ARRAY_H_ */
