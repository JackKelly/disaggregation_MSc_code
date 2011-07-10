#define BOOST_TEST_MODULE Array test
#define BOOST_TEST_DYN_LINK
#define GOOGLE_STRIP_LOG 4
#include "../src/Array.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE( rollingAvTest )
{
    const int SIZE = 10;
    int pop[SIZE] = {2,4,4,4,5,5,7,9,3,4};
    Array<int> raTest(SIZE, pop);
    RollingAvArray raArray;
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

    for (int i=0; i<SIZE; i++) {
        BOOST_CHECK_EQUAL( raArray[i] , answers[i]);
    }

}
