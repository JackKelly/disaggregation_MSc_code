/*
 * Array.h
 *
 *  Created on: 5 Jul 2011
 *      Author: jack
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#include "Common.h"
#include "Utils.h"
#include "GNUplot.h"
#include <iostream>
#include <glog/logging.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cassert>
#include <list>

/**
 * @todo break RollingAverage out into a new file
 */
template <class T>
class RollingAverage {
public:
    RollingAverage(const size_t _size) : size(_size), index(0), filled(0), full(0)
    {
        data = new T[size];
        for (size_t i=0; i<size; i++) {
            data[i] = 0;
        }
    }

    ~RollingAverage()
    {
        delete [] data;
    }

    void newValue(const T newVal)
    {
        data[index++] = newVal;
        if (index >= size)
            index = 0;
        if (!full) {
            if (filled++ > size)
                full = true;
        }
    }

    const double value() {
        T accumulator=0;
        for (size_t i=0; i<size; i++) {
            accumulator+=data[i];
        }
        return (double)accumulator/ (full ? size : filled);
    }
private:
    T * data;
    size_t size;
    size_t index;
    size_t filled;
    bool   full;
};

/**
 * Array class.  At it's core it's just a wrapper around a C-style array.
 * With lots of added functionality useful for signal processing.
 */
template <class T>
class Array {
protected:
    /********************
     * Member variables *
     ********************/
    T * data;
    size_t size;         /**< Number of members. size_t is 8bytes wide on x86_64 */
    size_t smoothing;    /**< Does this array represent data which has been smoothed? */
    Array<T> const * upstream; /**< If this array has been produced by processing a previous array then 'upstream' points to the upstream array. Else=0. */

public:
    /********************
     * Member functions *
     ********************/

    /**************** CONSTRUCTORS **************/

    Array()
    : data(0), size(0), smoothing(0), upstream(0)
    {}

    explicit Array(
            const size_t _size /**< Size of array */
            )
    : data(0), size(0), smoothing(0), upstream(0)
    {
        setSize(_size);
    }

    /**
     * Constructor for building an Array from an existing C-array
     */
    explicit Array(
            const size_t _size, /**< Size of array */
            const T * _data     /**< Pointer to C-style array */
            )
    : data(0), size(0), smoothing(0), upstream(0)
    {
        setSize(_size);
        for (size_t i=0; i<size; i++) {
            data[i] = _data[i];
        }
    }

    /**
     * Copy constructor
     */
    Array( const Array<T>& other )
    : data(0), size(0), smoothing(0), upstream(&other)
    {
        setSize(other.size);


        for (size_t i=0; i<size; i++) {
            data[i] = other[i];
        }
    }

    virtual ~Array()
    {
        if ( data != 0 ) {
            delete [] data;
        }
    }

    Array<T>& operator=(const Array<T>& source)
    {
        setSize(source.size);
        upstream = &source;
        for (size_t i=0; i<size; i++) {
            data[i] = source[i];
        }

        return *this;
    }

    /************* GETTERS AND SETTERS **************/

    /**
     * Mutable subscript operator.  Fast.  No range checking
     */
    T& operator[](const size_t i)
    {
        return data[i];
    }

    /**
     * Const subscript operator.  Fast.  No range checking
     */
    const T& operator[](const size_t i) const
    {
        return data[i];
    }

    void setSize(const size_t _size)
    {
        if (size == _size)
            return;

        size = _size;
        LOG(INFO) << "Setting array size = " << size;
        try {
            if ( data!=0 ) { // check if this has already been used
                LOG(INFO) << "Deleting existing Array struct to make way for a new one of size " << _size;
                delete [] data;
                data=0;
            }
            data = new T[size];
        } catch (std::bad_alloc& ba) {
            LOG(FATAL) << "Failed to allocate memory: " << ba.what();
        }
    }

    const size_t getSize() const
    {
        return size;
    }

    const size_t getSmooting() const
    {
        return smoothing;
    }

    Array<T>* getUpstream() const
    {
        return upstream;
    }

    /************* OTHER MEMBER FUNCTIONS ****************/

    bool operator==(const Array<T>& other)
    {
        if (size != other.size)
            return false;

        for (size_t i=0; i<size; i++) {
            if (data[i] != other[i])
                return false;
        }

        return true;
    }


    /**
     * Copy from source to this object, starting at cropFront index and
     * ignoring the last cropBack items from the source.
     *
     * @param source
     * @param cropFront
     * @param cropBack
     */
    void copyCrop(const Array<T>& source, const size_t cropFront, const size_t cropBack)
    {
        if ( source.size==0 || source.data==0 ) {
            LOG(WARNING) << "Nothing to copy.";
            return;
        }

        this->setSize( source.size - cropFront - cropBack );
        upstream = &source;

        for (size_t i = 0; i<size; i++ ) {
            data[i] = source[i+cropFront];
        }
    }

    void initAllEntriesTo(const T initValue)
    {
        upstream = 0;
        for (size_t i = 0; i<size; i++) {
            data[i] = initValue;
        }
    }

    void dumpToFile(const std::string& filename) const
    {
        LOG(INFO) << "Dumping Array to filename " << filename;
        std::fstream fs;
        fs.open( filename.c_str(), std::ifstream::out );
        if ( ! fs.good() ) {
            LOG(FATAL) << "Can't open " << filename;
        }
        fs << *this;
        fs.close();
    }

    /**
     * Returns a rolling average of same length as the original array.
     *
     * @todo this code should probably be removed and the functionality implemented useing the RollingAverage class.
     *
     * @param ra = Initally an empty Array<Sample_t>.  Returned with Rolling Averages.
     * @param length = number of items to use in the average.  Must be odd.
     */
    void rollingAv(Array<Sample_t> * ra, const size_t length=5) const
    {
        assert( length%2 );  // length must be odd
        assert( length>1 );
        assert( length<size);

        // setup ra
        ra->smoothing = length;
        ra->upstream = this;
        ra->setSize(this->size);

        size_t i;
        T accumulator, accumulatorDown;

        size_t middleOfLength = ((length-1)/2)+1;

        // First do the first (length-1)/2 elements of the Array<Sample_t>
        // and the last (length-1)/2 elements
        (*ra)[0] = data[0];
        for (i=1; i<(middleOfLength-1); i++) {
            accumulator = 0;
            accumulatorDown = 0;
            for (size_t inner=0; inner<=(i*2); inner++) {
                accumulator += data[inner];
                accumulatorDown += data[size-1-inner];
            }
            (*ra)[i] = (double)accumulator / ( (i*2) + 1 );
            (*ra)[size-1-i] = (double)accumulatorDown / ( (i*2) + 1 );
        }

        // Now do the main chunk of the array
        for (; i<=(size-middleOfLength); i++) {
            accumulator = 0;
            for (size_t inner=(i-middleOfLength+1); inner<(i+middleOfLength); inner++) {
                accumulator += data[inner];
            }
            (*ra)[i] = (double)accumulator/length;
        }

        // Do the last element of the array
        (*ra)[size-1] = data[size-1];
    }

    /**
     * Return the index of and the value of the largest element of the Array,
     * ignoring the members between 'start' and 'end'.
     *
     * @return maxValue
     */
    const size_t max(T* maxValue, const size_t start=0, size_t end=0) const
    {
        LOG(INFO) << "max(start=" << start << ",end=" << end << ")";
        if (end==0)
            end = size;

        assert( start <= end  );
        assert( start <  size );
        assert( end   <= size );

        if (start == end) {
            *maxValue = data[start];
            return start;
        }

        T maxSoFar=0;
        size_t indexOfMaxSoFar=start;

        for (size_t i=start; i<end; i++) {
            if (data[i] > maxSoFar) {
                maxSoFar = data[i];
                indexOfMaxSoFar = i;
            }
        }

        *maxValue = maxSoFar;
        return indexOfMaxSoFar;
    }

    /**
     * Find the max value outside of the mask.
     *
     * @return an index to the max value.  If there are two equal values
     * then we only return the first we come to.
     */
    const size_t max(
            T* maxValue, /**< Return the max value value */
            std::list<size_t>& mask ///< a list of (mask start index, mask end index)
                                    ///< pairs describing the mask.
                                    ///< The masks must not overlap.
                                    ///< The mask includes the start and end index.
            ) const
    {
        if ( mask.empty() ) {
            LOG(INFO) << "Mask is empty.";
            return max( maxValue, 0, size );
        }

        // check there are an even number of items in the list (start, end pairs)
        assert( ( mask.size() % 2 )==0 );

        mask.sort();
        for (std::list<size_t>::const_iterator it=mask.begin(); it!=mask.end(); it++) {
            LOG(INFO) << *it;
        }


        T maxSoFar=0, maxThisLoop=0;
        size_t indexOfMaxSoFar=0, indexOfMaxThisLoop=0, start=0, end;
        std::list<size_t>::const_iterator it=mask.begin();

        // Deal with the case where the start of the first mask is 0
        if (*it == 0) {
            it++;
            start = *(it++) + 1;
        }

        // Find the max between each mask
        while ( it != mask.end() ) {
            end = *(it++);

            // Deal with the case where 2 adjacent masks, mask A and mask B and A.end == B.start
            if ( end == (start-1) ) {
                start = *(it++) + 1;
                continue;
            }

            indexOfMaxThisLoop = max( &maxThisLoop, start, end );
            if (maxThisLoop > maxSoFar) {
                maxSoFar = maxThisLoop;
                indexOfMaxSoFar = indexOfMaxThisLoop;
            }
            start = *(it++) + 1; // "+1" so we exclude the end of the mask
            if (start >= size)
                break;
        }

        // Now get the max from the end of the last mask until the end of the array
        if (start < size) {
            indexOfMaxThisLoop = max( &maxThisLoop, start, size );
            if (maxThisLoop > maxSoFar) {
                maxSoFar = maxThisLoop;
                indexOfMaxSoFar = indexOfMaxThisLoop;
            }
        }

        *maxValue = maxSoFar;
        return indexOfMaxSoFar;
    }

    /**
     * Find where the peak plateaus or starts climbing again.
     *
     * @deprecated This doesn't work anywhere near as well as findPeaks().
     * But leaving this function in the code in case it comes in handy.
     *
     * @return 'start' and 'end' parameters are return parameters
     */
    void descendPeak(
            const size_t peak, /**< the index of the highest value in the peak */
            size_t* start, /**< return parameter = index of the start of the hill */
            size_t* end,  /**< return parameter = index of the end of the hill */
            size_t retries = 1  /**< the "tolerance" */
            )
    {
        size_t i = peak;
        size_t retriesRight = retries, retriesLeft = retries;

        // Descend right side of peak
        while ( true ) {
            i++;
            if ( i >= size ) {
                *end = size;
                break;
            }

            if ( data[i] >= data[i-1] ) {
                retriesRight--;

                if ( retriesRight == 0 ) {
                    *end = i;
                    break;
                }
            }
        }

        // Descend left side of peak
        i = peak;
        while ( true ) {
            i--;
            if ( i <= 0 ) {
                *start = 0;
                break;
            }

            if ( data[i] >= data[i+1] ) {
                retriesLeft--;

                if ( retriesLeft == 0 ) {
                    *start = i;
                    break;
                }
            }

        }
    }

    /**
     * Attempts to automatically find the peaks.
     *
     * Starts from end of array and works backwards.  At each step, calculates a rolling
     * average of the gradient.  If the value of this rolling average is above KNEE_GRAD_THRESHOLD
     * then it enters the 'ASCENDING' state... etc...
     */
    void findPeaks(
            std::list<size_t> * boundaries /**< output parameter */
            )
    {
        LOG(INFO) << "findPeaks...";
        const double KNEE_GRAD_THRESHOLD = 0.6;
        const double SHOULDER_GRAD_THRESHOLD = 0.6;
        const size_t RA_LENGTH = 19; /* Length of rolling average. Best if odd. */
        size_t middleOfRA;
        enum { NO_MANS_LAND, ASCENDING, PEAK, DESCENDING, UNSURE } state;
        state = NO_MANS_LAND;
        RollingAverage<int> gradientRA(RA_LENGTH);
        T kneeHeight=0, peakHeight=0, descent=0, ascent=0;

        // Load the Rolling Average array
        for (size_t i=(size-2); i>(size-RA_LENGTH); i--) {
            gradientRA.newValue( (int)(data[i] - data[i+1]) );
        }

        double * RA = new double[(size-RA_LENGTH)+1]; // Just used for data visualisation purposes

        // start at the end of the array, working backwards.
        for (size_t i=(size-RA_LENGTH); i>0; i--) {
            // keep a rolling average of the gradient
            gradientRA.newValue( (int)(data[i] - data[i+1]) );

            RA[i] = gradientRA.value();

            middleOfRA = i+((RA_LENGTH/2)+1);

            switch (state) {
            case NO_MANS_LAND:
                if (gradientRA.value() > KNEE_GRAD_THRESHOLD) {
                    // when the gradient goes over a certain threshold, mark that as ASCENDING,
                    //    and record index in 'boundaries' and kneeHeight
                    state=ASCENDING;
                    LOG(INFO) << "ASCENDING " << middleOfRA;
                    boundaries->push_front( middleOfRA );
                    kneeHeight = data[ middleOfRA ];
                }
                // TODO deal with descent from NO_MANS_LAND
                break;
            case ASCENDING:
                // when gradient drops to 0, state = PEAK, record peakHeight
                if (gradientRA.value() < SHOULDER_GRAD_THRESHOLD) {
                    state=PEAK;
                    LOG(INFO) << "PEAK " << middleOfRA;
                    peakHeight = data[ middleOfRA ];
                    ascent = peakHeight - kneeHeight;
                }
                break;
            case PEAK:
                // when gradient goes below a certain threshold, state = DESCENDING
                if (gradientRA.value() < -SHOULDER_GRAD_THRESHOLD) {
                    state=DESCENDING;
                    LOG(INFO) << "DESCENDING " << middleOfRA;
                }
                break;
            case DESCENDING:
                // when gradient goes to 0, check height.  If we've descended less than 30% our ascent height then don't mark
                //   in 'boundaries', instead re-mark as ASCENDING
                // else mark in 'boundaries' and state=NO_MANS_LAND
                if (gradientRA.value() > -KNEE_GRAD_THRESHOLD) {
                    // check how far we've descended from the shoulder
                    descent = peakHeight - data[ middleOfRA ];

                    if (descent < (0.3 * ascent)) {
                        state=UNSURE;
                        LOG(INFO) << "UNSURE " << middleOfRA;
                    } else {
                        state=NO_MANS_LAND;
                        LOG(INFO) << "NO_MANS_LAND " << middleOfRA;
                        boundaries->push_front( middleOfRA );
                    }
                }
                break;
            case UNSURE:
                // We were descending but hit a plateau prematurely.
                // Find out if we're descending or ascending
                if (gradientRA.value() < -SHOULDER_GRAD_THRESHOLD) {
                    state = DESCENDING;
                    LOG(INFO) << "DESCENDING " << middleOfRA;
                }

                if (gradientRA.value() > KNEE_GRAD_THRESHOLD) {
                    state = ASCENDING;
                    LOG(INFO) << "ASCENDING " << middleOfRA;
                }

                break;
            };
        }

        if (state != NO_MANS_LAND) {
            boundaries->push_front(0);
        }

        // For debugging purposes, dump rolling average data to file
        std::fstream ra_file;
        ra_file.open( "data/processed/RA_hist.dat", std::fstream::out);
        for (size_t i = 0; i<(size-RA_LENGTH); i++) {
            ra_file << RA[i] << std::endl;
        }
        ra_file.close();
        delete [] RA;

    }

    virtual void drawGraph(
            const std::string& name,
            const std::string& xlabel = "",
            const std::string& ylabel = "",
            const std::string& args = ""
            )
    {
        // Dump data to a .dat file
        const std::string dataFilename = DATA_OUTPUT_PATH + name + ".dat";
        dumpToFile( dataFilename );

        // Set plot variables
        GNUplot::PlotVars pv;
        pv.inFilename  = "1line";
        pv.outFilename = name;
        pv.title       = name;
        pv.xlabel      = xlabel;
        pv.ylabel      = ylabel;
        pv.plotArgs    = args;
        pv.data.push_back( GNUplot::Data( dataFilename, name ) );

        // Plot
        GNUplot::plot( pv );
    }

    /**
     * Load data from a CSV file with a single column.
     */
    void loadData(
            std::fstream& fs /**< An opened and valid file stream containing a CSV datafile. */
            )
    {
        setSize( Utils::countDataPoints( fs ) );

        int count = 0;
        char ch;
        while ( ! fs.eof() ) {
            ch = fs.peek();
            if ( isdigit(ch) ) {
                fs >> data[ count++ ];  // attempt to read a float from the file
            }
            fs.ignore( 255, '\n' );  // skip to next line
        }
        LOG(INFO) << "Entered " << count << " ints into data array.";
    }


    const size_t getNumLeadingZeros()
    {
        assert (data != 0);
        size_t count = 0;
        while ( data[count]==0 && count<size ) {
            count++;
        }
        return count;
    }

    const size_t getNumTrailingZeros()
    {
        assert (data != 0);
        size_t count = size-1;
        while ( data[count]==0 && count>0 ) {
            count--;
        }
        return (size-count)-1;
    }


    friend std::ostream& operator<<(std::ostream& o, const Array<T>& a)
    {
        for (size_t i=0; i<a.size; i++) {
            o << a[i] << std::endl;
        }
        return o;
    }
};

class Histogram : public Array<Histogram_t>
{
public:
    /**
     * Create a histogram from a sample array.
     *
     * @param hist = an empty Array<uint32_t> object
     */
    Histogram(const Array<Sample_t>& source)
    : Array<Histogram_t>()
    {
        upstream = (Array<Histogram_t>*)&source; /**< @todo this may not work! */

        setSize( MAX_WATTAGE ); // 1 Watt resolution

        initAllEntriesTo(0);

        for (size_t i=0; i<size; i++) {
            data[ Utils::roundToNearestInt( data[i] ) ]++;
        }
    }

    /**
     * Override because we need to convert 'upstream' back to the correct pointer type.
     */
    Array<Sample_t>* getUpstream()
    {
        return (Array<Sample_t>*)upstream;
    }
};

#endif /* ARRAY_H_ */
