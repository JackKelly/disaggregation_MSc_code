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
private:
    struct TimeAndPower {
        size_t timestamp;
        double meanPower;
        TimeAndPower( const size_t _timestamp, const double _meanPower )
        : timestamp(_timestamp), meanPower(_meanPower)
        {}
    };
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
        double avLikelihood;
        std::list< TimeAndPower > timeAndPower;

        friend std::ostream& operator<<(std::ostream& o, const DisagDataItem& ddi) {

            o << "  timestamp      = " << ddi.timestamp << std::endl
              << "  date           = " << ctime( (time_t*)(&ddi.timestamp) )
              << "  av likelihood  = " << ddi.avLikelihood << std::endl
              << "  duration       = " << ddi.duration << " seconds"
              << " (" << Utils::secondsToTime(ddi.duration) << ")" << std::endl
              << "  energy         = " << ddi.energy << " Joules"
              << " equivalent to " << ddi.energy / J_PER_KWH << " kWh";
            return o;
        }
    };

    const std::list<DisagDataItem> getStartTimes(
            const AggregateData& aggregateData,
            const bool keep_overlapping = false,
            const bool verbose = false
            );

    friend std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg );

private:
    /***********************************
     * P.S.G. GRAPH USED FOR TRAINING: *
     * (PSG = Power State Graph)       *
     ***********************************/
    struct PowerStateEdge; // forward declared to fix interdependency introduced by edgeHistory

    struct PowerStateVertex {
        Statistic<Sample_t> betweenSpikes; /**< @brief Stats taken for the entire
                                                period between spikes. Used for estimating
                                                energy consumption. */
        Statistic<Sample_t> postSpike; /**< @brief Stats taken for a
                                            few samples immediately after the spike. Used
                                            for determining vertices during training. */
    };


    /**
     * @brief A graph where each vertex(node) is a @c Statistic<Sample_t>
     *        and each edge is a @c PowerStateEdge .
     *
     * See <a href="http://www.boost.org/doc/libs/1_47_0/libs/graph/doc/bundles.html">boost::graph bundles tutorial</a>
     */
    typedef boost::adjacency_list<
            boost::vecS, boost::vecS, boost::directedS,  // vecS allows multiple edges between vertices
            PowerStateVertex, // our custom vertex (node) type
            PowerStateEdge    // our custom edge type
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
                        << "min="  << g[v].betweenSpikes.min << " \\n"
                        << "mean=" << g[v].betweenSpikes.mean << " \\n"
                        << "max=" << g[v].betweenSpikes.max << " \\n"
                        << "stdev=" << g[v].betweenSpikes.stdev << " \\n"
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

    static const size_t EDGE_HISTORY_SIZE = 5;
    std::list< PSGraph::edge_descriptor > edgeHistory; /**< @brief a "rolling" list storing
                                                            the previous few edges we've seen. */

    struct LikelihoodAndVertex {
        double likelihood;
        DisagTree::vertex_descriptor vertex;

        LikelihoodAndVertex()
        : likelihood(0), vertex(0)
        {}

        LikelihoodAndVertex(const DisagTree::vertex_descriptor& _vertex)
        : likelihood(0), vertex(_vertex)
        {}
    };

    std::list< std::list<LikelihoodAndVertex> > listOfPaths;

    Statistic< double > energyConsumption; /**< @brief Energy consumption in Joules
                                                obtained from training signatures */

    /****************************
     * PRIVATE MEMBER FUNCTIONS *
     ****************************/

    PSGraph::vertex_descriptor updateOrInsertVertex(
            const Signature& sig,
            const Statistic<Sample_t> postSpikePowerState,
            const Statistic<Sample_t> betweenSpikesPowerState,
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

    void printSpikeInfo(
            const std::list<Signature::Spike>::iterator spike,
            const size_t start,
            const size_t end,
            const Statistic<Sample_t>& before,
            const Statistic<Sample_t>& after,
            const Signature& sig
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
            const LikelihoodAndVertex cav,
            std::list<LikelihoodAndVertex> path = std::list<LikelihoodAndVertex>(0)
            );

    const DisagDataItem findBestPath(
            const DisagTree& disagTree,
            const size_t deviceStart,
            const bool verbose = false
            );

    void removeOverlapping(
            std::list<DisagDataItem> * disagList, /**< Input and output parameter */
            const bool verbose = false
            );

    void displayAndPlotDisagList(
            const std::list< DisagDataItem >& disagList
            ) const;

    const size_t indexOfNextSpike(
            const std::list<Signature::Spike>& spikes,
            std::list<Signature::Spike>::iterator spike,
            const Signature& sig
            ) const;


};

#endif /* POWERSTATEGRAPH_H_ */
