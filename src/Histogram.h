/*
 * Histogram.h
 *
 *  Created on: 9 Sep 2011
 *      Author: jack
 */

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include "Array.h"
#include "Common.h"

class Histogram : public Array<Histogram_t>
{
public:

    Histogram(
            const Array<Sample_t>& source,
            const size_t xaxis = MAX_WATTAGE
            );

    virtual const std::string getBaseFilename() const;

    virtual void drawGraph();

    const size_t getSizeOfSource() const;

protected:
    size_t sizeOfSource;

};



#endif /* HISTOGRAM_H_ */
