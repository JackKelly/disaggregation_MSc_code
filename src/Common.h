/*
 * common.h
 *
 *  Created on: 6 Jul 2011
 *      Author: jack
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <cstddef> // size_t
#include <string>
#include <list>


/******************************
 *        TYPEDEFS            *
 *                            *
 * Just the typedefs based on *
 * primitive types are listed *
 * here.                      *
 ******************************/

typedef double  Sample_t;     /**< @brief An individual sample (as recorded by a CurrentCost or WattsUp) */
typedef double  Histogram_t;  /**< @brief An individual histogram entry */

/*****************************
 *         CONSTS            *
 *****************************/

const int EXIT_ERROR = 1;
const size_t MAX_WATTAGE = 3500;
const std::string DATA_OUTPUT_PATH = "data/output/"; /**< @deprecated Will be replaced with config option */
const bool REMOVE_OVERLAPPING = true;

#endif /* COMMON_H_ */
