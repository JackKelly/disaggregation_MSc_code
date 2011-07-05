/*
 * Array.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include "Disaggregate.h"
#include <iostream>
#include <cstdlib> // for std::exit
#include <glog/logging.h>


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

    ~Array() {
        if ( value != 0 ) {
            delete [] value;
        }
    }

    void setSize(const int _size)
    {
        size = _size;
        try {
            LOG(INFO) << "Creating array with size = " << size;
            value = new T[size];
        } catch (std::bad_alloc& ba) {
            std::cerr << "Failed to allocate memory: " << ba.what() << std::endl;
            std::exit(EXIT_ERROR);
        }
    }
};

#endif /* ARRAY_H_ */
