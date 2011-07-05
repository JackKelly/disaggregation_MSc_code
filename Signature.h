/*
 * Signature.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include "Array.h"

class Signature {
public:
    Signature();
    virtual ~Signature();

private:

    /************************
     *  Member functions    *
     ************************/



    /************************
     *  Member variables    *
     ************************/

    Array rawReading;
    int samplePeriod;

};

#endif /* SIGNATURE_H_ */
