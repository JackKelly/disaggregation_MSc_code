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
#include <time.h>
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
        double energy;    /**< @brief energy consumed */
        double confidence;

        friend std::ostream& operator<<(std::ostream& o, const DisagDataItem& ddi) {

//            struct tm * t = localtime( &ddi.timestamp );

            o << "timestamp   = " << ddi.timestamp << std::endl
              << "date        = " << ctime( (time_t*)(&ddi.timestamp) )
              << "confidence  = " << ddi.confidence << std::endl
              << "duration    = " << ddi.duration << " seconds"
              << " (" << Utils::secondsToTime(ddi.duration) << ")" << std::endl
              << "energy      = " << ddi.energy << " Joules"
              << " equivalent to " << ddi.energy / 3600000 << " kWh";
            return o;
        }
    };

    const std::list<DisagDataItem> getStartTimes(
            const AggregateData& aggregateData, /**< A populated array of AggregateData */
            const bool verbose = false
            );

    friend std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg );

private:
    /***********************************
     * P.S.G. GRAPH USED FOR TRAINING: *
     * (PSG = Power State Graph)       *
     ***********************************/
    struct PowerStateEdge; // forward declared to fix interdependency introduced by edgeHistory

    /**
     * @brief A graph where each vertex(node) is a @c Statistic<Sample_t>
     *        and each edge is a @c PowerStateEdge .
     *
     * See <a href="http://www.boost.org/doc/libs/1_47_0/libs/graph/doc/bundles.html">boost::graph bundles tutorial</a>
     */
    typedef boost::adjacency_list<
//            boost::vecS, boost::vecS, boost::bidirectionalS,
//            boost::setS, boost::vecS, boost::directedS, // setS means we can only have single edges
            boost::vecS, boost::vecS, boost::directedS,  // vecS allows multiple edges between vertices
            Statistic<Sample_t>,   // our custom vertex (node) type
            PowerStateEdge         // our custom edge type
            > PSGraph;

    struct PowerStateEdge {
        Statistic<double> delta;
        Statistic<size_t> duration;
        size_t count; /**< @brief The number of times this edge has been
                                  traversed during training.  Used with @c totalCount
                                  to determine which edges are traversed most often.
                                  @todo remove if we're not using this. */
        std::list< PSGraph::edge_descriptor > edgeHistory; /**< @brief a "rolling" list storing
                                                                the previous few edges we've seen. */
    };


    typedef boost::graph_traits<PSGraph>::vertex_iterator PSG_vertex_iter;
    typedef boost::graph_traits<PSGraph>::edge_iterator PSG_edge_iter;
    typedef boost::graph_traits<PSGraph>::out_edge_iterator PSG_out_edge_iter;

    typedef boost::property_map<PSGraph, boost::vertex_index_t>::type PSG_vertex_index_map;
    typedef boost::property_map<PSGraph, boost::edge_index_t >::type PSG_edge_index_map;

    struct PSG_vertex_writer {

            PSG_vertex_writer(PSGraph& g_) : g (g_) {};

            template <class Vertex>
            void operator()(std::ostream& out, Vertex v) {
                    out << " [label=\""
                        << "min="  << g[v].min << " \\n"
                        << "mean=" << g[v].mean << " \\n"
                        << "max=" << g[v].max << " \\n"
                        << "stdev=" << g[v].stdev << " \\n"
                        << "\"]";
            };

            PSGraph g;
    };

    struct PSG_edge_writer {

            PSG_edge_writer(PSGraph& g_) : g (g_) {};

            template <class Edge>
            void operator()(std::ostream& out, Edge e) {
                    out << " [label=\""
                        << "dMean=" << g[e].delta.mean << " \\n"
                        << " dSD=" << g[e].delta.stdev << " \\n"
                        << " dMin=" << g[e].delta.min << " \\n"
                        << " dMax=" << g[e].delta.max << " \\n"
                        << " durMean=" << g[e].duration.mean << " \\n"
                        << " durSD=" << g[e].duration.stdev << " \\n"
                        << " durMin=" << g[e].duration.min << " \\n"
                        << " durMax=" << g[e].duration.max << " \\n" ;

                    for (std::list< PSGraph::edge_descriptor >::const_iterator
                            edge=g[e].edgeHistory.begin();
                            edge != g[e].edgeHistory.end(); edge++) {
                        out << " \\n" << *edge;
                    }

                    out << "\"]";
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
        PSGraph::edge_descriptor psgEdge; /**< @brief the PSG edge which got us to where we are. */
        std::list< PSGraph::edge_descriptor > edgeHistory; /**< @brief a "rolling" list storing
                                                                the previous few edges we've seen. */


        DisagVertex()
        : timestamp(0), meanPower(0), psgVertex(0)
        {}

        DisagVertex(
                const size_t _timestamp,
                const double _meanPower,
                const PSGraph::vertex_descriptor _psgVertex,
                const PSGraph::edge_descriptor _psgEdge
                )
        : timestamp(_timestamp), meanPower(_meanPower), psgVertex(_psgVertex), psgEdge(_psgEdge)
        {}
    };


    /**
     * @brief Tree structure used to keep track of all the
     * possible solutions during disaggregation.
     *
     * Implemented using the Boost Graph Library.
     */
    typedef boost::adjacency_list<
            boost::setS, boost::vecS, boost::bidirectionalS,
            DisagVertex,   // our custom vertex (node) type
            double          // our custom edge type (for storing probabilities)
            > DisagTree;

    typedef boost::graph_traits<DisagTree>::out_edge_iterator Disag_out_edge_iter;
    typedef boost::graph_traits<DisagTree>::edge_descriptor Disag_edge_desc;

    /**
     * @brief used for write_graphviz for DisagGraph.
     */
    struct Disag_vertex_writer {

            Disag_vertex_writer(DisagTree& g_) : g (g_) {};

            template <class Vertex>
            void operator()(std::ostream& out, Vertex e) {
                    out << " [label=\""
                        <<   "timestamp=" << g[e].timestamp-1310252400
                        << ", meanPower=" << g[e].meanPower
                        << "\"]";
            };

            DisagTree g;
    };

    /**
     * @brief used for write_graphviz for DisagGraph.
     */
    struct Disag_edge_writer {

            Disag_edge_writer(DisagTree& g_) : g (g_) {};

            template <class Edge>
            void operator()(std::ostream& out, Edge e) {
                    out << " [label=\""
                        <<  g[e]
                        << "\"]";
            };

            DisagTree g;
    };




    /****************************
     * PRIVATE MEMBER VARIABLES *
     ****************************/

    PSGraph powerStateGraph; /**< @brief store the power state graph learnt during training. */

    PSGraph::vertex_descriptor offVertex; /**< @todo we probably don't need this as the offVertex will probably always been vertex index 0. */

    size_t totalCount; /**< @brief the total number of times any edge
                                   has been traversed during training. */

    AggregateData const * aggData;

    static const size_t EDGE_HISTORY_SIZE = 4;
    std::list< PSGraph::edge_descriptor > edgeHistory; /**< @brief a "rolling" list storing
                                                            the previous few edges we've seen. */

    struct ConfidenceAndVertex {
        double confidence;
        DisagTree::vertex_descriptor vertex;

        ConfidenceAndVertex()
        : confidence(0), vertex(0)
        {}

        ConfidenceAndVertex(const DisagTree::vertex_descriptor& _vertex)
        : confidence(0), vertex(_vertex)
        {}
    };


    std::list< std::list<ConfidenceAndVertex> > listOfPaths;

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
            const double spikeDelta,
            const bool verbose = false
            );

    void updateEdges( const Signature& sig );

    const DisagDataItem initTraceToEnd(
            const AggregateData::FoundSpike& spike,
            const size_t deviceStart,
            const bool verbose = false // set true to see graphviz output of trees
            );

    void traceToEnd(
            DisagTree * disagGraph,
            const DisagTree::vertex_descriptor& startVertex,
            const size_t prevTimestamp,
            const bool verbose = false
            ) const;

    void addItemToEdgeHistory(
            const PSGraph::edge_descriptor& edge
            );

    const bool edgeListsAreEqual(
            const std::list< PSGraph::edge_descriptor >& a,
            const std::list< PSGraph::edge_descriptor >& b,
            const bool verbose = false
            ) const;

    std::list< PSGraph::edge_descriptor > getEdgeHistoryForVertex(
            const DisagTree& disagTree,
            const DisagTree::vertex_descriptor& startVertex
            ) const;

    void findListOfPathsThroughDisagTree(
            const DisagTree& disagTree,
            const DisagTree::vertex_descriptor vertex,
            const ConfidenceAndVertex cav,
            std::list<ConfidenceAndVertex> path = std::list<ConfidenceAndVertex>(0)
            );

    const DisagDataItem findBestPath(
            const DisagTree& disagTree,
            const size_t deviceStart,
            const bool verbose = false //
            );

    void removeOverlapping(
            std::list<DisagDataItem> * disagList, /**< Input and output parameter */
            const bool verbose = false
            );


};

#endif /* POWERSTATEGRAPH_H_ */
