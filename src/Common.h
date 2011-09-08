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

const size_t MAX_WATTAGE = 3500; /**< @brief Maximum wattage allowed in DEVICE_RAW_DATA files */
const int J_PER_KWH = 3600000;   /**< @brief Joules per kWh */

const std::string DATA_OUTPUT_PATH = "data/output/";
const std::string SIG_DATA_PATH    = "data/input/watts_up/";
const std::string AGG_DATA_PATH    = "data/input/current_cost/";
const std::string GNUPLOT_SET_TERMINAL = "set terminal svg size 1200 800; set samples 1001";
//const std::string GNUPLOT_SET_TERMINAL = "set terminal epslatex solid colour size 17cm,10cm";
const std::string GNUPLOT_OUTPUT_FILE_EXTENSION = "svg";
//const std::string GNUPLOT_OUTPUT_FILE_EXTENSION = "tex";

#endif /* COMMON_H_ */
