/*
 * PowerStateSequence.h
 *
 *  Created on: 26 Jul 2011
 *      Author: jack
 */

#ifndef POWERSTATESEQUENCE_H_
#define POWERSTATESEQUENCE_H_

#include <list>
#include <string>
#include "Statistic.h"

typedef Statistic<Sample_t>     PowerState_t;  /**< @todo should PowerState inherit from Statistic? */
typedef std::list<PowerState_t> PowerStates_t;

struct PowerStateSequenceItem {
    PowerStates_t::const_iterator powerState; /**< @brief An iterator "pointer" to a powerState  */
    size_t startTime; /**< @brief in seconds  */
    size_t endTime;   /**< @brief in seconds  */
    Sample_t delta;   /**< @brief the delta corresponding to this power state sequence item  */
    PowerStateSequenceItem(): startTime(0), endTime(0), delta(0) {}
};

class PowerStateSequence : public std::list<PowerStateSequenceItem> {
public:
    void dumpToFile(const std::string details = "") const;
    void setDeviceName( const std::string& _deviceName );
    void plotGraph() const;
    const std::string getBaseFilename() const;
private:
    std::string deviceName;
};

#endif /* POWERSTATESEQUENCE_H_ */
