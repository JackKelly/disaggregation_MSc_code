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
typedef std::list<PowerState_t> PowerStates_t; /**< @todo should PowerStates_t be its own class, inheriting from std::list?  Then we could have a drawGraph function for PowerStates. */

struct PowerStateSequenceItem {
    PowerStates_t::const_iterator powerState; /**< @todo Does the same iterator point to the same element, no matter if an item is subsequently entered before the item?  And does the iterator point to the correct item even after copying?  */
    size_t startTime; /**< in seconds  */
    size_t endTime;   /**< in seconds  */
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
