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


/**
 * Very simple Array struct
 */
template <class T>
struct Array {
    T * value;
    int size;

    Array() : value(0), size(0)
    {}

    Array(const int _size)
    {
        setSize(_size);
    }

    ~Array()
    {
        if ( value != 0 ) {
            delete [] value;
        }
    }

    void setSize(const int _size)
    {
        size = _size;
        LOG(INFO) << "Creating array with size = " << size;
        try {
            if ( value!=0 ) { // check if this has already been used
                delete [] value;
                value=0;
            }
            value = new T[size];
        } catch (std::bad_alloc& ba) {
            LOG(FATAL) << "Failed to allocate memory: " << ba.what();
        }
    }

    void copyCrop(const Array<T>& source, const int cropFront, const int cropBack)
    {
        if ( source.size==0 || source.value==0 ) {
            LOG(WARNING) << "Nothing to copy.";
            return;
        }

        if ( this->size==0 || this->value==0 ) {
            this->setSize( source.size - cropFront - cropBack );
        }

        for (int i = 0; i<size; i++ ) {
            value[i] = source.value[i+cropFront];
        }
    }

    friend std::ostream& operator<<(std::ostream& o, const Array<T>& a)
    {
        for (int i=0; i<a.size; i++) {
            o << a.value[i] << std::endl;
        }
        return o;
    }
};

#endif /* ARRAY_H_ */
