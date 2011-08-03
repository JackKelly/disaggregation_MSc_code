/*
 * PowerStateGraph.h
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#ifndef POWERSTATEGRAPH_H_
#define POWERSTATEGRAPH_H_

#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include "Statistic.h"
#include "Common.h"    // for Sample_t
#include "Signature.h"

class PowerStateGraph {
public:
    PowerStateGraph();
    virtual ~PowerStateGraph();

    void updateVertices( const Signature& sig );

private:
    struct PowerStateEdge {
        Statistic<double> delta;
        Statistic<size_t> duration;
    };

    typedef boost::adjacency_list<
            boost::vecS, boost::vecS, boost::bidirectionalS,
            Statistic<Sample_t>, // our vertex (node) type
            PowerStateEdge> graph;
};

#endif /* POWERSTATEGRAPH_H_ */
