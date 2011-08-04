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
#include <ostream>
#include "Statistic.h"
#include "Common.h"    // for Sample_t
#include "Signature.h"

class PowerStateGraph {
public:
    PowerStateGraph();
    virtual ~PowerStateGraph();

    void updateVertices( const Signature& sig );

    void updateEdges( const Signature& sig );

    friend std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg );

private:
    /************************
     * STRUCTS AND TYPEDEFS *
     ************************/
    struct PowerStateEdge {
        Statistic<double> delta;
        Statistic<size_t> duration;
    };

    /**
     * @brief A graph where each vertex(node) is a @c Statistic<Sample_t>
     *        and each edge is a @c PowerStateEdge .
     *
     * See <a href="http://www.boost.org/doc/libs/1_47_0/libs/graph/doc/bundles.html">boost::graph bundles tutorial</a>
     */
    typedef boost::adjacency_list<
            boost::vecS, boost::vecS, boost::bidirectionalS,
            Statistic<Sample_t>,   // our custom vertex (node) type
            PowerStateEdge         // our custom edge type
            > Graph;

    typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;

    typedef boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;

    /************************
     * MEMBER VARIABLES     *
     ************************/

    Graph graph;

    Graph::vertex_descriptor offVertex; /**< @todo we probably don't need this as the offVertex will probably always been vertex index 0. */

    /************************
     * MEMBER FUNCTIONS      *
     ************************/

    void updateOrInsertVertex(
            const Statistic<Sample_t>& stat,
            const Signature& sig,
            const size_t start,
            const size_t end
        );

};

#endif /* POWERSTATEGRAPH_H_ */