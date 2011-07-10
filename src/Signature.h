/*
 * Signature.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include "Array.h"
#include <fstream>

typedef double SigArrayDataType;
typedef Array<SigArrayDataType> SigArray;

class Signature {
public:
    Signature( const char* filename, const int _samplePeriod );
    virtual ~Signature();

    void downSample( SigArray * output, const int newPeriod );

    const SigArray& getSigArray() const;

private:

    /************************
     *  Member functions    *
     ************************/
    void editRawReading();

    void openFile( std::ifstream& fs, const char* filename );

    void loadData( std::ifstream& fs, SigArray* data );

    int  countDataPoints( std::ifstream& fs );

    void cropAndStore( const SigArray& data );

    const int findNumLeadingZeros( const SigArray& data );

    const int findNumTrailingZeros( const SigArray& data );

    /************************
     *  Member variables    *
     ************************/

    SigArray rawReading;
    int samplePeriod;

};

#endif /* SIGNATURE_H_ */
