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
    Signature( const char* filename, const size_t _samplePeriod );
    virtual ~Signature();

    void downSample( SigArray * output, const size_t newPeriod );

    const SigArray& getSigArray() const;

private:

    /************************
     *  Member functions    *
     ************************/
    void editRawReading();

    void openFile( std::ifstream& fs, const char* filename );

    void loadData( std::ifstream& fs, SigArray* data );

    const size_t  countDataPoints( std::ifstream& fs ) const;

    void cropAndStore( const SigArray& data );

    const size_t findNumLeadingZeros( const SigArray& data );

    const size_t findNumTrailingZeros( const SigArray& data );

    /************************
     *  Member variables    *
     ************************/

    SigArray rawReading;
    size_t samplePeriod;

};

#endif /* SIGNATURE_H_ */
