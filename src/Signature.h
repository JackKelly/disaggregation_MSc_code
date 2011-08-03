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
    /*****************
     *    STRUCTS    *
     *****************/

    /**
     * Recording the value and location of a Spike, e.g. a Spike in the gradient of a signature
     */
    struct Spike {
        size_t index;    /**< Location to find this Spike.     */
        size_t duration; /**< Length of spike in samples.      */
        double delta;    /**< Value of Spike at this location. */

        /**
         * Comparison function. Useful for sorting lists of spikes into descending order of absolute magnitude.
         */
        const static bool compareAbsValueDesc( const Spike first, const Spike second )
        {
            return fabs(first.delta) > fabs(second.delta);
        }

        const static bool compareIndexAsc( const Spike first, const Spike second )
        {
            return first.index < second.index;
        }
    };

    /*******************
     * FUNCTIONS       *
     *******************/

    /***************************************/
    /** @name Constructors and destructors */
    ///@{

    Signature(
            const char* filename,
            const size_t _samplePeriod,
            const std::string _deviceName,
            const size_t _sigID = 0,
            const size_t cropFront = 0,
            const size_t cropBack = 0
            );

    virtual ~Signature();

    ///@}

    /******************************/
    /** @name Getters and setters */
    ///@{
    const size_t getSamplePeriod() const;

    const PowerStates_t& getPowerStates();

    const PowerStateSequence& getPowerStateSequence();

    const std::list<Spike> getGradientSpikesInOrder() const;
    ///@}

    /************************/
    /** @name Graph drawing */
    ///@{
    void drawHistWithStateBars(
            const Histogram& hist
            ) const;

    virtual void drawGraph( const std::string details = "" ) const;
    ///@}

    /********************/
    /** @name Smoothing */
    ///@{

    void downSample( Array<Sample_t> * output, const size_t newPeriod ) const;

    ///@}

    /*****************
     * STATIC CONSTS *
     *****************/
    static const size_t PREPROCESSING_DATA_SMOOTHING = 1;

private:

    /************************
     *  Member functions    *
     ************************/

    /*********************************/
    /** @name Power state processing */
    ///@{

    void updatePowerStates();

    PowerStates_t::const_iterator getPowerState( const Sample_t sample ) const;

    const std::string getStateBarsBaseFilename() const;

    void fillGapsInPowerStates(
            const Histogram& hist
            );

    ///@}

    /******************************************/
    /** @name Power state sequence processing */
    ///@{

    void updatePowerStateSequence();

    const std::list<Spike> getMergedSpikes() const;

    ///@}

    /************************
     *  Member variables    *
     ************************/
    size_t samplePeriod;
    const size_t sigID; /**< Each Device can have multiple signatures. A Device's first sig gets a sigID of 0, the next gets a sigID of 1 etc. */
    PowerStates_t powerStates;
    PowerStateSequence powerStateSequence;

};

#endif /* SIGNATURE_H_ */
