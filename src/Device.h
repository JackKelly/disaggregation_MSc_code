/*
 * Device.h
 *
 *  Created on: 7 Jul 2011
 *      Author: jack
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <list>
#include <vector>
#include <string>
#include "Signature.h"
#include "Statistic.h"
#include "PowerStateSequence.h"
#include "AggregateData.h"
#include "PowerStateGraph.h"

/**
 * @brief Class for representing "devices" (e.g. appliances like "dish washer", "lamp" etc)
 */
class Device {
public:
    /***************************************/
    /** @name Constructors and destructors */
    ///@{

    Device(const std::string _name);
    virtual ~Device();

    ///@}

    /******************************************************************/
    /** @name General functions used by all disaggregation approaches */
    ///@{

    void loadSignatures(
            const std::vector< std::string >& sigFiles,
            const size_t cropFront,
            const size_t cropBack
            );

    const std::string getName() const;
    ///@}

    /*****************************************************/
    /** @name Functions used by the "histogram" approach */
    ///@{
    void getPowerStatesAndSequence();

    const std::list<size_t> getStartTimes( const AggregateData& ) const;

    const std::list<Signature::Spike> getSalientSpikes() const;
    ///@}

    /***********************************************/
    /** @name Functions used by the "LMS" approach */
    ///@{

    std::list<size_t> findAlignment(
            const AggregateData& aggregateData
            );
    ///@}


    /**************************************************************/
    /** @name Functions used by the "graphs and spikes" approach */
    ///@{
    void trainPowerStateGraph();

    PowerStateGraph& getPowerStateGraph();
    ///@}

private:
    /************************
     *  Member functions    *
     ************************/

    /*****************************************************/
    /** @name Private functions used by the "histogram" approach */
    ///@{
    void updatePowerStates();
    void updatePowerStateSequence();
    ///@}

    void loadCurrentCostData(std::fstream& fs, Array<AggregateSample> * aggData);

    const double LMS(
            const size_t agOffset,
            const Array<AggregateSample>& aggData,
            const Array<Sample_t>& sigArray,
            const size_t aggDataSamplePeriod);

    /************************
     *  Member variables    *
     ************************/
    std::string name;
    PowerStates_t powerStates;
    std::vector<Signature*> signatures;
    PowerStateSequence powerStateSequence;
    struct IndexAndDelta {
        size_t index;
        size_t delta;
    };
    PowerStateGraph powerStateGraph;

};

#endif /* DEVICE_H_ */
