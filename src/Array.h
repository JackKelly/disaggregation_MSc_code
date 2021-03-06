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
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cassert>
#include <list>


/**
 * @brief Array class.  At its core it's just a wrapper around a C-style array.
 * With lots of added functionality for signal processing.
 *
 * @todo should Array inherit from <a href="http://www.cplusplus.com/reference/std/valarray/">valarray</a>. ?
 */
template <class T>
class Array {
protected:
    /********************
     * Member variables *
     ********************/
    T * data;
    size_t size;         /**< @brief Number of members. size_t is 8bytes wide on x86_64 */
    size_t smoothing;    /**< @brief Does this array represent data which has been smoothed? */
    size_t upstreamSmoothing; /**< @brief If this array has been produced by processing
                                   a previous array, and that array was smoothed then
                                   'upstreamSmoothing' records the smoothing of the upstream array. Else=0. */
    std::string deviceName; /**< @brief Most arrays are associated with a device.  If not, just leave blank. */

public:
    /********************
     * Member functions *
     ********************/

    /***************************************/
    /** @name Constructors and destructors */
    ///@{

    Array()
    : data(0), size(0), smoothing(0), upstreamSmoothing(0), deviceName("")
    {}

    explicit Array(
            const size_t _size /**< Size of array */
            )
    : data(0), size(0), smoothing(0), upstreamSmoothing(0), deviceName("")
    {
        setSize(_size);
    }

    /**
     * @brief Constructor for building an Array from an existing C-array
     */
    explicit Array(
            const size_t _size, /**< Size of array */
            const T * _data     /**< Pointer to C-style array */
            )
    : data(0), size(0), smoothing(0), upstreamSmoothing(0), deviceName("")
    {
        setSize(_size);
        for (size_t i=0; i<size; i++) {
            data[i] = _data[i];
        }
    }

    /**
     * @brief Copy constructor
     */
    Array( const Array<T>& other )
    : data(0), size(0), smoothing(other.getSmoothing()), upstreamSmoothing(other.getUpstreamSmoothing()), deviceName(other.deviceName)
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

    ///@}

    /******************************************/
    /** @name Operators                       */
    ///@{

    Array<T>& operator=(const Array<T>& source)
    {
        setSize(source.size);
        smoothing = source.getSmoothing();
        upstreamSmoothing = source.getUpstreamSmoothing();
        deviceName = source.deviceName;
        for (size_t i=0; i<size; i++) {
            data[i] = source[i];
        }

        return *this;
    }

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
    ///@}

    /******************************/
    /** @name Getters and setters */
    ///@{

    /**
     * @brief Mutable subscript operator.  Fast.  No range checking
     */
    T& operator[](const size_t i)
    {
        return data[i];
    }

    /**
     * @brief Const subscript operator.  Fast.  No range checking
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

        try {
            if ( data!=0 ) { // check if this has already been used
                delete [] data;
                data=0;
            }
            data = new T[size];
        } catch (std::bad_alloc& ba) {
            Utils::fatalError(std::string("Failed to allocate memory: ") + ba.what() );
        }
    }

    const size_t getSize() const
    {
        return size;
    }

    const size_t getSmoothing() const
    {
        return smoothing;
    }

    /**
     * @brief Used simply to keep tracking of smoothing parameters throughout the workflow.
     */
    void setSmoothing (const size_t _smoothing)
    {
        smoothing = _smoothing;
    }

    /**
     * @see @c upstreamSmoothing
     */
    const size_t getUpstreamSmoothing() const
    {
        return upstreamSmoothing;
    }

    void setUpstreamSmoothing( const size_t _upstreamSmoothing)
    {
        upstreamSmoothing = _upstreamSmoothing;
    }

    void setDeviceName( const std::string _deviceName)
    {
        deviceName=_deviceName;
    }

    const std::string getDeviceName() const
    {
        return deviceName;
    }

    /**
     * @brief Initialise all array entries to a single value.
     * Does not reset 'deviceName' or 'smoothing' or 'upstreamSmoothing'.
     */
    void setAllEntriesTo(const T initValue)
    {
        for (size_t i = 0; i<size; i++) {
            data[i] = initValue;
        }
    }

    ///@}


    /************************************/
    /** @name File IO and graph drawing */
    ///@{

    void dumpToFile(
            const std::string& filename /**< Excluding path and suffix.  DATA_OUTPUT_PATH and .dat will be added. */
            ) const
    {
        std::string fullFilename = DATA_OUTPUT_PATH + filename + ".dat";
        std::fstream fs;
        fs.open( fullFilename.c_str(), std::ifstream::out );
        if ( ! fs.good() ) {
            Utils::fatalError("Can't open " + fullFilename);
        }
        fs << *this;
        fs.close();
    }

    const std::string getSmoothedGradOfHistFilename() const
    {
        return deviceName + "-smoothedGradOfHist-" + Utils::size_t_to_s( HIST_GRADIENT_RA_LENGTH );
    }

    virtual const std::string getBaseFilename() const
    {
        std::string baseFilename = deviceName +
                (smoothing ? ("-Smoothing" + Utils::size_t_to_s(smoothing)) : "") +
                (upstreamSmoothing ? ("-UpstreamSmoothing" + Utils::size_t_to_s(upstreamSmoothing)) : "");
        return baseFilename;
    }


    virtual void drawGraph(
            const std::string details,  /**< Type of graph e.g. "gradient".  NOT device name, which gets added automatically. */
            const std::string xlabel = "",
            const std::string ylabel = "",
            const std::string args   = ""
            ) const
    {
        // Dump data to a .dat file

        const std::string baseFilename = getBaseFilename() + details;

        dumpToFile( baseFilename );

        // Set plot variables
        GNUplot::PlotVars pv;
        pv.inFilename  = "1line";
        pv.outFilename = baseFilename;
        pv.title       = baseFilename;
        pv.xlabel      = xlabel;
        pv.ylabel      = ylabel;
        pv.plotArgs    = args;
        pv.data.push_back( GNUplot::PlotData( baseFilename, baseFilename ) );

        // Plot
        GNUplot::plot( pv );
    }

    /**
     * @brief Load data from a CSV file with a single column.
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
    }


    ///@}

    /******************************/
    /** @name Smoothing           */
    ///@{

    /**
     * @brief Returns a rolling average of same length as the original array.
     */
    void rollingAv(
            Array<Sample_t> * destination,   /**< Initially an empty Array<Sample_t>.  Returned with Rolling Averages. */
            const size_t RAlength=5 /**< number of items to use in the average.  Must be odd. */
            ) const
    {
        // setup ra
        destination->setSmoothing( RAlength );
        destination->setUpstreamSmoothing( smoothing );
        destination->setDeviceName( deviceName );
        destination->setSize(this->size);

        for (size_t i=0; i<size; i++) {
            (*destination)[i] = rollingAv(i, RAlength);
        }
    }

    /**
     * @return an average of the RAlength items nearest to i
     */
    T rollingAv(
            const size_t i,  /**< Index of the smoothed rolling average. */
            size_t RAlength  /**< Length of rolling average. Must be odd. Can be 1. */
            ) const
    {
        assert( (RAlength%2)!=0 );
        assert( RAlength < size );

        if (i==0 || i==(size-1))
            return data[i];

        const size_t eitherSide = RAlength/2;

        if ( i < eitherSide )
            RAlength = (i*2)+1;

        if ( i > (size - eitherSide - 1) )
            RAlength = ((size-i)*2)-1;

        T accumulator = 0;
        for (size_t j=(i-(RAlength/2)); j<(i+(RAlength/2)+1); j++) {
            accumulator += data[j];
        }

        return accumulator/RAlength;
    }
    ///@}


    /**********************************************************/
    /** @name Functions used mostly by the histogram approach */
    ///@{

    /**
     * @brief Return the index of and the value of the largest element of the Array,
     * ignoring the members between 'start' and 'end'.
     *
     * @return maxValue
     */
    const size_t max(T* maxValue, const size_t start=0, size_t end=0) const
    {
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
     * @brief Find the max value outside of the mask.
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
            return max( maxValue, 0, size );
        }

        // check there are an even number of items in the list (start, end pairs)
        assert( ( mask.size() % 2 )==0 );

        mask.sort();

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
     * @brief Find where the peak plateaus or starts climbing again.
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

    void getDelta(
            Array<T> * grad, /**< Return parameter.
                                  Comes in as an instantiated but empty @C Array.
                                  Leaves as an @c Array of deltas. */
            const double multiplier=1 /**< Useful for negating the gradient,
                                           for example (by setting @c multiplier=-1 ). */
            ) const
    {
        grad->setSize( size );
        grad->setDeviceName ( deviceName );
        grad->setUpstreamSmoothing( smoothing );
        grad->setSmoothing( 0 );

        for (size_t i=0; i<(size-1); i++) {
            (*grad)[i] = getDelta(i) * multiplier;
        }

        (*grad)[size-1] = 0;
    }

    /**
     * @return \code data[i+1] - data[i] \endcode
     */
    const T getDelta(
            const size_t i
            ) const
    {
        if ( i>(size-2) )
            return 0;
        else
            return (data[i+1] - data[i]);
    }

    static const size_t HIST_GRADIENT_RA_LENGTH = 17; /**< @brief Length of rolling average of histogram gradient.
                                                          Best if odd. */
    /**
     * @brief Attempts to automatically find the peaks.
     *
     * Starts from end of array and works backwards.  At each step, calculates a rolling
     * average of the gradient.  If the value of this rolling average is above KNEE_GRAD_THRESHOLD
     * then it enters the 'ASCENDING' state... etc...
     *
     * @todo needs to work with relative histogram values (maybe this should be done in the Histogram class?)
     */
    void findPeaks(
            std::list<size_t> * boundaries /**< output parameter */
            )
    {
        const bool STUPIDLY_VERBOSE_LOGGING = true;
        const double KNEE_GRAD_THRESHOLD = 0.00014;
        const double SHOULDER_GRAD_THRESHOLD = 0.00014;
        enum { NO_MANS_LAND, ASCENDING, PEAK, DESCENDING, UNSURE } state;
        state = NO_MANS_LAND;
        T kneeHeight=0, peakHeight=0, descent=0, ascent=0;

        Array<Sample_t> gradient;
        getDelta( &gradient, -1 );

        // Construct an array of smoothed gradients
        Array<Sample_t> smoothedGrad;
        gradient.rollingAv( &smoothedGrad, HIST_GRADIENT_RA_LENGTH);

        // start at the end of the array, working backwards.
        for (size_t i=(size-HIST_GRADIENT_RA_LENGTH); i>0; i--) {

            switch (state) {
            case NO_MANS_LAND:
                if (smoothedGrad[i] > KNEE_GRAD_THRESHOLD) {
                    // when the gradient goes over a certain threshold, mark that as ASCENDING,
                    //    and record index in 'boundaries' and kneeHeight
                    state=ASCENDING;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "ASCENDING " << i << std::endl;
                    boundaries->push_front( i );
                    kneeHeight = data[ i ];
                }
                // TODO deal with descent from NO_MANS_LAND
                break;
            case ASCENDING:
                // when gradient drops to 0, state = PEAK, record peakHeight
                if (smoothedGrad[i] < SHOULDER_GRAD_THRESHOLD) {
                    state=PEAK;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "PEAK " << i << std::endl;
                    peakHeight = data[ i ];
                    ascent = peakHeight - kneeHeight;
                }
                break;
            case PEAK:
                // when gradient goes below a certain threshold, state = DESCENDING
                if (smoothedGrad[i] < -SHOULDER_GRAD_THRESHOLD) {
                    state=DESCENDING;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "DESCENDING " << i << std::endl;
                }
                if (data[i] == 0) {
                    state=NO_MANS_LAND;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "NO_MANS_LAND " << i << std::endl;
                    boundaries->push_front( i );
                }

                break;
            case DESCENDING:
                // when gradient goes to 0, check height.  If we've descended less than 30% our ascent height then don't mark
                //   in 'boundaries', instead re-mark as ASCENDING
                // else mark in 'boundaries' and state=NO_MANS_LAND
                if (smoothedGrad[i] > -KNEE_GRAD_THRESHOLD  || data[i]==0) {
                    // check how far we've descended from the shoulder
                    descent = peakHeight - data[ i ];

                    if (descent < (0.3 * ascent)) {
                        state=UNSURE;
                        if (STUPIDLY_VERBOSE_LOGGING) std::cout << "UNSURE " << i << std::endl;
                    } else {
                        state=NO_MANS_LAND;
                        if (STUPIDLY_VERBOSE_LOGGING) std::cout << "NO_MANS_LAND " << i << std::endl;
                        boundaries->push_front( i );
                    }
                }
                break;
            case UNSURE:
                // We were descending but hit a plateau prematurely.
                // Find out if we're descending or ascending
                if (smoothedGrad[i] < -SHOULDER_GRAD_THRESHOLD) {
                    state = DESCENDING;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "DESCENDING " << i << std::endl;
                    break;
                }

                if (smoothedGrad[i] > KNEE_GRAD_THRESHOLD) {
                    state = ASCENDING;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "ASCENDING " << i << std::endl;
                    break;
                }

                if (data[i]==0) {
                    state=NO_MANS_LAND;
                    if (STUPIDLY_VERBOSE_LOGGING) std::cout << "NO_MANS_LAND " << i << std::endl;
                    boundaries->push_front( i );
                    break;
                }

                break;
            };
        }

        if (state != NO_MANS_LAND) {
            boundaries->push_front(0);
        }

        // For data visualisation purposes, dump rolling average data to file
        smoothedGrad.dumpToFile( getSmoothedGradOfHistFilename() );
    }
    ///@}

    /******************************/
    /** @name Cropping            */
    ///@{

    /**
     * @brief Copy from source to this object, starting at cropFront index and
     * ignoring the last cropBack items from the source.
     */
    void copyCrop(const Array<T>& source, const size_t cropFront, const size_t cropBack)
    {
        if ( source.size==0 || source.data==0 ) {
            return;
        }

        this->setSize( (source.size - cropFront - cropBack) + 3 ); // "+3" for the added 0 at the front and two 0s at the end (important for the Signature::getGradientSpike code))
        upstreamSmoothing = source.getUpstreamSmoothing();
        smoothing = source.getSmoothing();
        deviceName = source.deviceName;

        data[0] = 0;
        for (size_t i = 0; i<(size-2); i++ ) {
            data[i+1] = source[i+cropFront];
        }
        data[size-1] = 0;
        data[size-2] = 0;
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
    ///@}


    friend std::ostream& operator<<(std::ostream& o, const Array<T>& a)
    {
        for (size_t i=0; i<a.size; i++) {
            o << a[i] << std::endl;
        }
        return o;
    }
};

#endif /* ARRAY_H_ */
