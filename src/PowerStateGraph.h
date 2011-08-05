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
#include <list>
#include <ostream>
#include "Statistic.h"
#include "Common.h"    // for Sample_t
#include "Signature.h"
#include "AggregateData.h"

class PowerStateGraph {
public:
    PowerStateGraph();
    virtual ~PowerStateGraph();

    void update(
            const Signature& sig,
            const bool verbose = false
            );

    void writeGraphViz(std::ostream& out);

    const std::list<size_t> getStartTimes(
            const AggregateData& aggregateData, /**< A populated array of AggregateData */
            const bool verbose = true
            ) const;

    friend std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg );

private:
    /************************
     * STRUCTS AND TYPEDEFS *
     ************************/
    struct PowerStateEdge {
        Statistic<double> delta;
        Statistic<size_t> duration;
        size_t count; /**< @brief The number of times this edge has been
                                  traversed during training.  Used with @c totalCount
                                  to determine which edges are traversed most often. */
    };

    /**
     * @brief A vertex for the directed acylic graphs used by getStartTimes()
     */
    struct DissagVertex {
        size_t timestamp; /**< UNIX timestamp taken from AggregateData. */
        double meanPower; /**< The mean power used between this vertex and the previous one. */
    };

    /**
     * @brief A graph where each vertex(node) is a @c Statistic<Sample_t>
     *        and each edge is a @c PowerStateEdge .
     *
     * See <a href="http://www.boost.org/doc/libs/1_47_0/libs/graph/doc/bundles.html">boost::graph bundles tutorial</a>
     */
    typedef boost::adjacency_list<
//            boost::vecS, boost::vecS, boost::bidirectionalS,
            boost::setS, boost::vecS, boost::directedS,
            Statistic<Sample_t>,   // our custom vertex (node) type
            PowerStateEdge         // our custom edge type
            > PSGraph;

    /**
     * @brief Directed Acyclic Graph (DAG) used during disaggregation.
     */
    typedef boost::adjacency_list<
            boost::setS, boost::vecS, boost::directedS,
            DissagVertex,   // our custom vertex (node) type
            boost::property<boost::edge_weight_t, double>
            > DAGGraph;

    typedef boost::graph_traits<PSGraph>::vertex_iterator PSG_vertex_iter;
    typedef boost::graph_traits<PSGraph>::edge_iterator PSG_edge_iter;

    typedef boost::property_map<PSGraph, boost::vertex_index_t>::type PSG_vertex_index_map;
    typedef boost::property_map<PSGraph, boost::edge_index_t >::type PSG_edge_index_map;

    struct my_edge_writer {

            my_edge_writer(PSGraph& g_) : g (g_) {};

            template <class Edge>
            void operator()(std::ostream& out, Edge e) {
                    out << " [label=\""
                        << "dMean=" << g[e].delta.mean
                        << " dSD=" << g[e].delta.stdev
//                        << " dMin=" << g[e].delta.min
//                        << " dMax=" << g[e].delta.max
                        << " durMean=" << g[e].duration.mean
//                        << " durMin=" << g[e].duration.min
//                        << " durMax=" << g[e].duration.max
                        << "\"]";
            };

            PSGraph g;
    };


    /************************
     * MEMBER VARIABLES     *
     ************************/

    PSGraph graph;

    PSGraph::vertex_descriptor offVertex; /**< @todo we probably don't need this as the offVertex will probably always been vertex index 0. */

    size_t totalCount; /**< @brief the total number of times any edge
                                   has been traversed during training. */

    /************************
     * MEMBER FUNCTIONS      *
     ************************/

    PSGraph::vertex_descriptor updateOrInsertVertex(
            const Statistic<Sample_t>& stat,
            const Signature& sig,
            const size_t start,
            const size_t end,
            const bool verbose = false
        );

    PSGraph::vertex_descriptor mostSimilarVertex(
            bool * success,
            const Statistic<Sample_t>& stat,
            const double ALPHA = 0.0000005
        ) const;

    const bool rejectSpike(
            const Statistic<Sample_t>& before,
            const Statistic<Sample_t>& after,
            const bool verbose = false
            ) const;

    void updateOrInsertEdge(
            const PSGraph::vertex_descriptor& before,
            const PSGraph::vertex_descriptor& after,
            const size_t sampleSinceLastSpike,
            const double spikeDelta
            );

    void updateEdges( const Signature& sig );

};

#endif /* POWERSTATEGRAPH_H_ */
