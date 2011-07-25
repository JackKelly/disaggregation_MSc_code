/*
 * Signature.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include "Array.h"
#include "Common.h"
#include "Statistic.h"
#include <list>
#include <fstream>

/**
 * @todo should inherit from Array<Sample_t>, shouldn't it?
 */
class Signature {
public:

    Signature(
            const char* filename,
            const size_t _samplePeriod,
            const size_t _sigID = 0,
            const size_t cropFront = 0,
            const size_t cropBack = 0
            );

    virtual ~Signature();

    void downSample( Array<Sample_t> * output, const size_t newPeriod );

    const Array<Sample_t>& getRawReading() const;

    const PowerStates_t& getPowerStates( const size_t rollingAvLength);

    const size_t getSamplePeriod();

    void drawHistWithStateBars(
            const Array<Histogram_t>&,
            const size_t rollingAvLength,
            const std::string& deviceName
            );

private:

    /************************
     *  Member functions    *
     ************************/
    void editRawReading();

    const size_t findNumLeadingZeros( const Array<Sample_t>& data );

    const size_t findNumTrailingZeros( const Array<Sample_t>& data );

    void findPowerStates( const size_t rollingAvLength );

    void fillGapsInPowerStates( const Array<Histogram_t>& hist );

    /************************
     *  Member variables    *
     ************************/

    Array<Sample_t> rawReading;
    size_t samplePeriod;
    PowerStates_t powerStates; /**< @todo saving powerStates is a bit pointless; just process it on demand */
    const size_t sigID; /**< Each Device can have multiple signatures. A Device's first sig gets a sigID of 0, the next gets a sigID of 1 etc. */

};

#endif /* SIGNATURE_H_ */
