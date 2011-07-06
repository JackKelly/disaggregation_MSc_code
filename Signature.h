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

typedef double SigArrayValueType;
typedef Array<SigArrayValueType> SigArray;

class Signature {
public:
    Signature( const char* filename, const int _samplePeriod );
    virtual ~Signature();

    void resample( SigArray& output, const int newPeriod );

private:

    /************************
     *  Member functions    *
     ************************/
    void editRawReading();

    void openFile( std::ifstream& fs, const char* filename );

    void loadData( std::ifstream& fs, SigArray* data );

    int  countDataPoints( std::ifstream& fs );

    int  roundToNearestInt( const double input );

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
