/*
 * Signature.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include "Array.h"
#include "Statistic.h"
#include <list>
#include <fstream>

typedef double Sample_t;
typedef Array<Sample_t> SigArray_t;
typedef std::list<Statistic<Histogram_t>> PowerStates_t;

class Signature {
public:

    Signature( const char* filename, const size_t _samplePeriod );

    virtual ~Signature();

    void downSample( SigArray_t * output, const size_t newPeriod );

    const SigArray_t& getSigArray() const;

    const PowerStates_t& getPowerStates();

    void drawGraphWithCandlesticks( const HistogramArray_t& );

private:

    /************************
     *  Member functions    *
     ************************/
    void editRawReading();

    void openFile( std::ifstream& fs, const char* filename );

    void loadData( std::ifstream& fs, SigArray_t* data );

    const size_t  countDataPoints( std::ifstream& fs ) const;

    void cropAndStore( const SigArray_t& data );

    const size_t findNumLeadingZeros( const SigArray_t& data );

    const size_t findNumTrailingZeros( const SigArray_t& data );

    void findPowerStates();

    /************************
     *  Member variables    *
     ************************/

    SigArray_t rawReading;
    size_t samplePeriod;
    PowerStates_t powerStates;

};

#endif /* SIGNATURE_H_ */
