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
#include "PowerStateSequence.h"
#include <list>
#include <fstream>

/**
 *
 */
class Signature : public Array<Sample_t> {
public:
    Signature(
            const char* filename,
            const size_t _samplePeriod,
            const std::string _deviceName,
            const size_t _sigID = 0,
            const size_t cropFront = 0,
            const size_t cropBack = 0
            );

    virtual ~Signature();

    void downSample( Array<Sample_t> * output, const size_t newPeriod ) const;

    const PowerStates_t getPowerStates( const size_t rollingAvLength) const;

    const size_t getSamplePeriod() const;

    void drawHistWithStateBars(
            const Histogram& hist
            ) const;

    virtual void drawGraph( const std::string details = "" ) const;

    void dumpPowerStateSequenceToFile() const;

    const PowerStates_t& getPowerStates();

    const PowerStateSequence& getPowerStateSequence();

    /*****************
     * STATIC CONSTS *
     *****************/
    static const size_t PREPROCESSING_DATA_SMOOTHING = 1;

private:

    /************************
     *  Member functions    *
     ************************/
    void updatePowerStates();

    void fillGapsInPowerStates(
            const Histogram& hist
            );

    void updatePowerStateSequence();

    PowerStates_t::const_iterator getPowerState( const Sample_t sample ) const;

    const std::string getStateBarsBaseFilename() const;

    /************************
     *  Member variables    *
     ************************/
    size_t samplePeriod;
    const size_t sigID; /**< Each Device can have multiple signatures. A Device's first sig gets a sigID of 0, the next gets a sigID of 1 etc. */
    PowerStates_t powerStates;
    PowerStateSequence powerStateSequence;

};

#endif /* SIGNATURE_H_ */
