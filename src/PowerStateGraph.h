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

    struct DisagDataItem {
        size_t timestamp; /**< @brief UNIX timestamp for start time */
        size_t duration;
        double energy; /**< @brief energy consumed */
        double confidence;
    };

    const std::list<DisagDataItem> getStartTimes(
            const AggregateData& aggregateData, /**< A populated array of AggregateData */
            const bool verbose = true
            );

    friend std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg );

private:
    /***********************************
     * P.S.G. GRAPH USED FOR TRAINING: *
     * (PSG = Power State Graph)       *
     ***********************************/
    struct PowerStateEdge {
        Statistic<double> delta;
        Statistic<size_t> duration;
        size_t count; /**< @brief The number of times this edge has been
                                  traversed during training.  Used with @c totalCount
                                  to determine which edges are traversed most often. */
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

    typedef boost::graph_traits<PSGraph>::vertex_iterator PSG_vertex_iter;
    typedef boost::graph_traits<PSGraph>::edge_iterator PSG_edge_iter;
    typedef boost::graph_traits<PSGraph>::out_edge_iterator PSG_out_edge_iter;

    typedef boost::property_map<PSGraph, boost::vertex_index_t>::type PSG_vertex_index_map;
    typedef boost::property_map<PSGraph, boost::edge_index_t >::type PSG_edge_index_map;

    struct PSG_edge_writer {

            PSG_edge_writer(PSGraph& g_) : g (g_) {};

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


    /**********************************
     * GRAPH USED FOR DISAGGREGATION: *
     **********************************/

    /**
     * @brief A vertex for the directed acylic graphs used by getStartTimes()
     */
    struct DisagVertex {
        size_t timestamp; /**< @brief UNIX timestamp taken from AggregateData. */
        double meanPower; /**< @brief Mean power used between this and the previous vertex. */
        PSGraph::vertex_descriptor psgVertex; /**< @brief link to vertex in Power State Graph. */
        bool deadEnd; /**< @todo remove this if I implement unwinding. */

        DisagVertex()
        : timestamp(0), meanPower(0), psgVertex(0), deadEnd(0)
        {}

        DisagVertex(
                const size_t _timestamp,
                const double _meanPower,
                const PSGraph::vertex_descriptor _psgVertex,
                const bool _deadEnd = false
                )
        : timestamp(_timestamp), meanPower(_meanPower), psgVertex(_psgVertex), deadEnd(_deadEnd)
        {}
    };


    /**
     * @brief Tree structure used to keep track of all the
     * possible solutions during disaggregation.
     *
     * Implemented using the Boost Graph Library.
     */
    typedef boost::adjacency_list<
            boost::setS, boost::vecS, boost::directedS,
            DisagVertex,   // our custom vertex (node) type
            double          // our custom edge type (for storing probabilities)
            > DisagGraph;

    typedef boost::graph_traits<DisagGraph>::out_edge_iterator DAG_out_edge_iter;
    typedef boost::graph_traits<DisagGraph>::edge_descriptor DAG_edge_desc;

    /**
     * @brief used for write_graphviz for DisagGraph.
     */
    struct Disag_vertex_writer {

            Disag_vertex_writer(DisagGraph& g_) : g (g_) {};

            template <class Vertex>
            void operator()(std::ostream& out, Vertex e) {
                    out << " [label=\""
                        <<   "timestamp=" << g[e].timestamp-1310252400
                        << ", meanPower=" << g[e].meanPower
                        << "\"]";
            };

            DisagGraph g;
    };

    /**
     * @brief used for write_graphviz for DisagGraph.
     */
    struct Disag_edge_writer {

            Disag_edge_writer(DisagGraph& g_) : g (g_) {};

            template <class Edge>
            void operator()(std::ostream& out, Edge e) {
                    out << " [label=\""
                        <<  g[e]
                        << "\"]";
            };

            DisagGraph g;
    };




    /****************************
     * PRIVATE MEMBER VARIABLES *
     ****************************/

    PSGraph powerStateGraph; /**< @brief store the power state graph learnt during training. */

    PSGraph::vertex_descriptor offVertex; /**< @todo we probably don't need this as the offVertex will probably always been vertex index 0. */

    size_t totalCount; /**< @brief the total number of times any edge
                                   has been traversed during training. */

    AggregateData const * aggData;

    /****************************
     * PRIVATE MEMBER FUNCTIONS *
     ****************************/

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

    const DisagDataItem initTraceToEnd(
            const AggregateData::FoundSpike& spike,
            const size_t deviceStart
            ) const;

    void traceToEnd(
            DisagGraph * disagGraph,
            DisagGraph::vertex_descriptor startVertex
            ) const;


};

#endif /* POWERSTATEGRAPH_H_ */
