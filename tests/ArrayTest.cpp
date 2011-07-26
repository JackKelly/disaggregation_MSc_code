#define BOOST_TEST_MODULE Array test
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/Array.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <list>

template <class T>
void checkWrite(Array<T>& a)
{
    T value;
    // Check we can write to every element
    for (size_t i=0; i<a.getSize(); i++) {
        value = i/5; // arbitrary way to generate a double
        a[i] = value;
        BOOST_CHECK_EQUAL( a[i], value );
    }
}

BOOST_AUTO_TEST_CASE( defaultConstructor )
{
    Array<int> ia;
    BOOST_CHECK_EQUAL( ia.getSize(), 0 );

    Array<double> da;
    BOOST_CHECK_EQUAL( da.getSize(), 0 );
}

BOOST_AUTO_TEST_CASE( sizeConstructor )
{
    Array<int> ia(100);
    BOOST_CHECK_EQUAL( ia.getSize(), 100 );
    checkWrite( ia );

    Array<double> da(1000);
    BOOST_CHECK_EQUAL( da.getSize(), 1000 );
    checkWrite( da );
}

BOOST_AUTO_TEST_CASE( copyConstructor )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> src(SIZE, pop);

    Array<int> dest = src;

    // Check dest are src are equal
    BOOST_CHECK( src == dest );
}

BOOST_AUTO_TEST_CASE( testAssignmentAndEqualityoperators )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> src(SIZE, pop);

    Array<int> dest, dest2;
    dest2 = dest = src;

    // Check dest are src are equal
    BOOST_CHECK( src == dest );
    BOOST_CHECK( dest2 == dest );
    BOOST_CHECK( dest2 == src );

}

BOOST_AUTO_TEST_CASE( rollingAvTest )
{
    const size_t SIZE = 10;
    Sample_t pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<Sample_t> raTest(SIZE, pop);
    Array<Sample_t> raArray;
    raTest.rollingAv(&raArray,7);

    double answers[SIZE] = {
            2,
            3.3333333333333335,
            3.8,
            4.4285714285714288,
            5.4285714285714288,
            5.2857142857142856,
            5.2857142857142856,
            5.6,
            5.333333333333333,
            4
    };

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_CLOSE( raArray[i] , answers[i], 0.0001);
    }
}

BOOST_AUTO_TEST_CASE( copyCropTest1 )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> arrayTestSrc(SIZE, pop);

    Array<int> arrayTestDest;

    arrayTestDest.copyCrop(arrayTestSrc, 3, 4);

    int answers[3] = {4, 5, 5};

    for (size_t i=0; i<3; i++) {
        BOOST_CHECK_EQUAL( arrayTestDest[i] , answers[i]);
    }
}

BOOST_AUTO_TEST_CASE( copyCropTest2 )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> arrayTestSrc(SIZE, pop);

    Array<int> arrayTestDest;

    arrayTestDest.copyCrop(arrayTestSrc, 0, 4);

    int answers[] = {2, 4, 4, 4, 5, 5};

    for (size_t i=0; i<6; i++) {
        BOOST_CHECK_EQUAL( arrayTestDest[i] , answers[i]);
    }
}

BOOST_AUTO_TEST_CASE( copyCropTest3 )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> arrayTestSrc(SIZE, pop);

    Array<int> arrayTestDest;

    arrayTestDest.copyCrop(arrayTestSrc, 0, 0);

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_EQUAL( arrayTestDest[i] , arrayTestSrc[i]);
    }
}

BOOST_AUTO_TEST_CASE( setAllEntriesToTest )
{
    const size_t SIZE = 10;
    Array<int> arrayTest(SIZE);

    arrayTest.setAllEntriesTo(0);

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_EQUAL( arrayTest[i] , 0);
    }
}

BOOST_AUTO_TEST_CASE( setSizeTest )
{
    Array<double> da;
    da.setSize(100);

    // Check the size is correct
    BOOST_CHECK_EQUAL( da.getSize(), 100);

    // Check we can write to every element
    checkWrite( da );

    // Now change the size
    da.setSize(1000);

    // Check the size is correct
    BOOST_CHECK_EQUAL( da.getSize(), 1000);

    // Check we can write to every element
    checkWrite( da );
}

BOOST_AUTO_TEST_CASE( histogram )
{
    const size_t SIZE = 10;
    Sample_t pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<Sample_t> arrayTestSrc(SIZE, pop);
    Histogram hist (arrayTestSrc);
                          // 0 1   2    3    4    5   6   7   8   9
    Histogram_t answers[] = {0,0, 0.1, 0.1, 0.4, 0.2, 0, 0.1, 0, 0.1};

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_CLOSE(answers[i], hist[i], 0.01);
    }
}

BOOST_AUTO_TEST_CASE( max )
{
    const size_t SIZE = 10;

                  // 0 1 2 3 4 5 6 7 8 9
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> arrayTestSrc(SIZE, pop);

    int maxValue;
    size_t indexOfMax = arrayTestSrc.max( &maxValue, 0, SIZE );

    // Trying the max() overloaded with a mask
    BOOST_CHECK_EQUAL(indexOfMax, 7);
    BOOST_CHECK_EQUAL(maxValue, 9);

    size_t maskCArray[] = {3,5, 7,9};
    std::list<size_t> mask(maskCArray, maskCArray+4);
    indexOfMax = arrayTestSrc.max( &maxValue, mask );

    BOOST_CHECK_EQUAL(indexOfMax, 6);
    BOOST_CHECK_EQUAL(maxValue, 7);

    // A different mask
    size_t maskCArray2[] = {3,5, 8,9};
    std::list<size_t> mask2(maskCArray2, maskCArray2+4);
    indexOfMax = arrayTestSrc.max( &maxValue, mask2 );

    BOOST_CHECK_EQUAL(indexOfMax, 7);
    BOOST_CHECK_EQUAL(maxValue, 9);

    // A different mask
    size_t maskCArray3[] = {3,7, 8,9};
    std::list<size_t> mask3(maskCArray3, maskCArray3+4);
    indexOfMax = arrayTestSrc.max( &maxValue, mask3 );

    BOOST_CHECK_EQUAL(indexOfMax, 1);
    BOOST_CHECK_EQUAL(maxValue, 4);

    // A different mask, starting at 0
                   // 0 1 2 3 4 5 6 7 8 9
    int pop2[SIZE] = {2,9,4,4,5,5,7,9,3,4};
    Array<int> arrayTestSrc2(SIZE, pop2);

    size_t maskCArray4[] = {0,3, 8,9};
    std::list<size_t> mask4(maskCArray4, maskCArray4 + 4);
    indexOfMax = arrayTestSrc2.max( &maxValue, mask4 );

    BOOST_CHECK_EQUAL(indexOfMax, 7);
    BOOST_CHECK_EQUAL(maxValue, 9);

    // A different mask, starting at zero, only 1 mask
    size_t maskCArray5[] = {0,3};
    std::list<size_t> mask5(maskCArray5, maskCArray5 + 2);
    indexOfMax = arrayTestSrc2.max( &maxValue, mask5 );

    BOOST_CHECK_EQUAL(indexOfMax, 7);
    BOOST_CHECK_EQUAL(maxValue, 9);


}

BOOST_AUTO_TEST_CASE( descendPeak )
{
    const size_t SIZE = 23;
                   //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
    int pop[SIZE] = {6,5,4,3,2,3,5,7,8,9,8,7,6,5,4,3,5,4,2,1,1,1,0};
    Array<int> arrayTestSrc(SIZE, pop);

    size_t start, end;
    arrayTestSrc.descendPeak(9, &start, &end);

    BOOST_CHECK_EQUAL( start, 3);
    BOOST_CHECK_EQUAL( end  ,16);
}


BOOST_AUTO_TEST_CASE( rollingAvsize_tsize_t )
{
    const size_t SIZE = 10;
    Sample_t pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<Sample_t> arrayTestSrc(SIZE, pop);

    Sample_t answers[] = {
            2,
            3.3333333333333335,
            4,
            4.333333333333333,
            4.666666666666667,
            5.666666666666667,
            7,
            6.333333333333333,
            5.333333333333333,
            4
    };

    Sample_t answers2[] = {
            2,
            3.33333,
            3.8,
            4.42857,
            4.77778,
            5,
            5.28571,
            5.6,
            5.33333,
            4
    };

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_CLOSE(arrayTestSrc.rollingAv(i, 3), answers[i] , 0.0001);
        BOOST_CHECK_CLOSE(arrayTestSrc.rollingAv(i, 1), arrayTestSrc[i], 0.0001);
        BOOST_CHECK_CLOSE(arrayTestSrc.rollingAv(i, 9), answers2[i], 0.001);
    }
}
BOOST_AUTO_TEST_CASE( rollingAvsize_tsize_t2 )
{
    const size_t SIZE = 11;
    Sample_t pop[SIZE] = {2,4,4,4,5,5,7,9,3,4,3};
    Array<Sample_t> arrayTestSrc(SIZE, pop);

    Sample_t answers[] = {
            2,
            3.3333333333333335,
            4,
            4.333333333333333,
            4.666666666666667,
            5.666666666666667,
            7,
            6.333333333333333,
            5.333333333333333,
            3.333333333333333,
            3
    };

    Sample_t answers2[] = {
            2,
            3.33333,
            3.8,
            4.42857,
            4.77778,
            5,
            4.8888888889,
            5.1428571429,
            5.2,
            3.3333333333,
            3
    };

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_CLOSE(arrayTestSrc.rollingAv(i, 3), answers[i] , 0.001);
        BOOST_CHECK_CLOSE(arrayTestSrc.rollingAv(i, 1), arrayTestSrc[i], 0.0001);
        BOOST_CHECK_CLOSE(arrayTestSrc.rollingAv(i, 9), answers2[i], 0.001);
    }
}
