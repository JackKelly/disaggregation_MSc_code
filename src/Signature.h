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
typedef Statistic<Sample_t> PowerState_t;
typedef std::list<PowerState_t> PowerStates_t;

class Signature {
public:

    Signature( const char* filename, const size_t _samplePeriod,
               const size_t cropFront = 0, const size_t cropBack = 0 );

    virtual ~Signature();

    void downSample( SigArray_t * output, const size_t newPeriod );

    const SigArray_t& getRawReading() const;

    const PowerStates_t& getPowerStates( const size_t rollingAvLength);

    const size_t getSamplePeriod();

    void drawGraphWithStateBars( const HistogramArray_t&, const size_t rollingAvLength );

private:

    /************************
     *  Member functions    *
     ************************/
    void editRawReading();

    const size_t findNumLeadingZeros( const SigArray_t& data );

    const size_t findNumTrailingZeros( const SigArray_t& data );

    void findPowerStates( const size_t rollingAvLength );

    void fillGapsInPowerStates( const HistogramArray_t& hist );

    /************************
     *  Member variables    *
     ************************/

    SigArray_t rawReading;
    size_t samplePeriod;
    PowerStates_t powerStates;

};

#endif /* SIGNATURE_H_ */
