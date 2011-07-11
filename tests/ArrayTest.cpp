#define BOOST_TEST_MODULE Array test
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/Array.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

template <class T>
void checkWrite(Array<T>& a)
{
    T value;
    // Check we can write to every element
    for (size_t i=0; i<a.size; i++) {
        value = i/5; // arbitrary way to generate a double
        a[i] = value;
        BOOST_CHECK_EQUAL( a[i], value );
    }
}

BOOST_AUTO_TEST_CASE( defaultConstructor )
{
    Array<int> ia;
    BOOST_CHECK_EQUAL( ia.size, 0 );
    BOOST_CHECK_EQUAL( ia.data, (int*)0 );

    Array<double> da;
    BOOST_CHECK_EQUAL( da.size, 0 );
    BOOST_CHECK_EQUAL( da.data, (double*)0 );
}

BOOST_AUTO_TEST_CASE( sizeConstructor )
{
    Array<int> ia(100);
    BOOST_CHECK_EQUAL( ia.size, 100 );
    BOOST_CHECK( ia.data != (int*)0 );
    checkWrite( ia );

    Array<double> da(1000);
    BOOST_CHECK_EQUAL( da.size, 1000 );
    BOOST_CHECK( da.data != (double*)0 );
    checkWrite( da );
}

BOOST_AUTO_TEST_CASE( copyConstructor )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> src(SIZE, pop);

    Array<int> dest = src;

    // Check the copy constructor hasn't simply assigned dest.data = src.data
    BOOST_CHECK ( src.data != dest.data );

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

    // Check the copy constructor hasn't simply assigned dest.data = src.data
    BOOST_CHECK ( src.data != dest.data );
    BOOST_CHECK ( dest2.data != dest.data );
    BOOST_CHECK ( dest2.data != src.data );

    // Check dest are src are equal
    BOOST_CHECK( src == dest );
    BOOST_CHECK( dest2 == dest );
    BOOST_CHECK( dest2 == src );

}

BOOST_AUTO_TEST_CASE( rollingAvTest )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> raTest(SIZE, pop);
    RollingAv_t raArray;
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
        BOOST_CHECK_EQUAL( raArray[i] , answers[i]);
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

BOOST_AUTO_TEST_CASE( initAllEntriesToTest )
{
    const size_t SIZE = 10;
    Array<int> arrayTest(SIZE);

    arrayTest.initAllEntriesTo(0);

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_EQUAL( arrayTest[i] , 0);
    }
}

BOOST_AUTO_TEST_CASE( setSizeTest )
{
    Array<double> da;
    da.setSize(100);

    // Check the size is correct
    BOOST_CHECK_EQUAL( da.size, 100);

    // Check we can write to every element
    checkWrite( da );

    // Now change the size
    da.setSize(1000);

    // Check the size is correct
    BOOST_CHECK_EQUAL( da.size, 1000);

    // Check we can write to every element
    checkWrite( da );
}

BOOST_AUTO_TEST_CASE( histogram )
{
    const size_t SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> arrayTestSrc(SIZE, pop);
    Histogram_t hist(10);
    arrayTestSrc.histogram( &hist );
                       // 0 1 2 3 4 5 6 7 8 9
    uint32_t answers[] = {0,0,1,1,4,2,0,1,0,1};

    for (size_t i=0; i<SIZE; i++) {
        BOOST_CHECK_EQUAL(answers[i], hist[i]);
    }
}
