/*
 * PowerStateGraph.cpp
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#include "PowerStateGraph.h"
#include <iostream>
#include <list>
#include <boost/graph/graphviz.hpp>
#include <cstdio> // sprintf

using namespace std;

PowerStateGraph::PowerStateGraph()
{
    using namespace boost;

    totalCount = 0;

    // add a vertex to represent "off"
    offVertex = add_vertex(graph);
    graph[offVertex] = Statistic<Sample_t>(0);

}

PowerStateGraph::~PowerStateGraph()
{
    // TODO Auto-generated destructor stub
}

/**
 * Update or initialise Power State Graph.
 */
void PowerStateGraph::update( const Signature& sig )
{
    Statistic<Sample_t> powerState;

    // get the gradient spikes for the signature
    list<Signature::Spike> spikes = sig.getGradientSpikesInOrder();
    list<Signature::Spike>::iterator spike;
    size_t indexOfLastAcceptedSpike = 0;
    Graph::vertex_descriptor afterVertex, prevAcceptedVertex=offVertex;

    // take just the top ten (whilst ordered by absolute value)
    if (spikes.size() > 100) {
        list<Signature::Spike>::iterator it = spikes.begin();
        advance( it, 100 );
        spikes.erase( it, spikes.end() );
    }

    // re-order by index (i.e. by time)
    spikes.sort( Signature::Spike::compareIndexAsc );

    // for each spike, locate the samples immediately before and immediately after the spike
    const size_t WINDOW = 8; // how far either side of the spike will we look?
    for (spike = spikes.begin(); spike!=spikes.end(); spike++) {
        cout << "SPIKE: index=" << spike->index << ", delta=" << spike->delta << ", duration=" << spike->duration << endl;

        size_t start = ((spike->index > WINDOW) ?
                (spike->index - WINDOW) : 0 );

        size_t end = spike->index + WINDOW + spike->duration + 1;
        if (end > sig.getSize())
            end = sig.getSize();

        Statistic<Sample_t> before(sig, start, spike->index);
        Statistic<Sample_t> after(sig, (spike->index + spike->duration + 1), end);

        cout << "Before=" << before << endl
             << "After =" << after  << endl;

        if (rejectSpike(before, after) ||
            start < indexOfLastAcceptedSpike)  // check the spikes aren't too close
            cout << " REJECT";
        else {
            cout << " KEEP";

            afterVertex =
                    updateOrInsertVertex(  after, sig, (spike->index + spike->duration + 1), end);

            if (prevAcceptedVertex != afterVertex) {
                updateOrInsertEdge( prevAcceptedVertex, afterVertex,
                        (spike->index - indexOfLastAcceptedSpike), spike->delta );
            }

            prevAcceptedVertex = afterVertex;
            indexOfLastAcceptedSpike = spike->index;
        }
        cout << endl;

        for (size_t i=start; i<end; i++ ) {
            cout << i << "\t" << sig[i];

            if (i < spike->index)
                cout << " before";
            else if (i == spike->index)
                cout << " <--SPIKE";
            else if (i < (spike->index + spike->duration))
                cout << " <--Spike continues";
            else
                cout << " after";

            cout << endl;
        }
    }
}

const bool PowerStateGraph::rejectSpike(
        const Statistic<Sample_t>& before,
        const Statistic<Sample_t>& after
        ) const
{
    // if either 'before' or 'after' has a stdev greater than
    // its mean then reject
    if ( (before.stdev > (before.mean)) ||
         ( after.stdev > ( after.mean))   ) {
        cout << "stdev too big";
        return true;
    }


    if (Utils::within(before.mean, after.mean, Utils::largest(before.stdev, after.stdev)*2) ||
        Utils::within(before.mean, after.mean, Utils::largest(before.mean, after.mean)*0.2)     ) {
        cout << "means too close";
        return true;
    }

    // if we get to here then don't reject
    return false;
}

/**
 * @brief Attempts to find an existing vertex which is
 * statistically similar to @c stat.  If an existing similar
 * vertex is found then the vertex's stats are updated
 * with the new data points. If a similar vertex is not found,
 * and new vertex is inserted.
 */
PowerStateGraph::Graph::vertex_descriptor PowerStateGraph::updateOrInsertVertex(
        const Statistic<Sample_t>& stat, /**< stat to find or insert in graph vertices */
        const Signature& sig, /**< source of the raw data */
        const size_t start,   /**< start of data window */
        const size_t end      /**< end of data window */
    )
{

    bool foundSimilar; // return param for mostSimilarVertex()

    Graph::vertex_descriptor vertex = mostSimilarVertex( &foundSimilar, stat );

    cout << " mostSimlar = " << graph[vertex] << endl;

    if ( foundSimilar ) {
        if (vertex != offVertex) {
            cout << endl
                    << "Updating existing vertex: " << endl
                    << "    start=" << start << ", end=" << end << endl
                    << "    existing vertex = " << graph[vertex] << endl
                    << "    new stat        = " << stat << endl;;
            graph[vertex].update( sig, start, end );
            cout << "    merged          = " << graph[vertex] << endl;
        }
    } else {
        // then add a new vertex
        vertex = add_vertex(graph);
        graph[vertex] = Statistic<Sample_t>( sig, start, end );
        cout << endl
             << "Adding new vertex: " << endl
             << "    start=" << start << ", end=" << end << endl
             << "    vertex = " << graph[vertex] << endl
             << "    stat   = " << stat << endl;;
    }
    return vertex;

}

/**
 * @brief Find the statistically most similar vertex to @c stat.
 *
 * @return vertex descriptor of best fit.
 *         @c success is also used as a return parameter.
 */
PowerStateGraph::Graph::vertex_descriptor PowerStateGraph::mostSimilarVertex(
        bool * success, /**< return parameter.  Did we find a satisfactory match? */
        const Statistic<Sample_t>& stat, /**< stat to find in graph vertices */
        const double ALPHA /**< significance level (what constitutes as a "satisfactory" match?) */
    ) const
{
    Graph::vertex_descriptor vertex=0;
    std::pair<vertex_iter, vertex_iter> vp;
    double tTest, highestTTest=0, diff, lowestDiff = std::numeric_limits<double>::max();

    // Find the best fit
    for (vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
        // t test
/*        tTest = stat.tTest( graph[*vp.first] );
        cout << "tTest=" << tTest << "  " << graph[*vp.first] << endl;
        if (tTest > highestTTest) {
            highestTTest = tTest;
            vertex = *vp.first;
        }
*/

        // Now compare means.  In the vast majority of the cases, comparing
        // means will produce the same best match as the tTest.
        diff = fabs( graph[*vp.first].mean - stat.mean );
        if (diff < lowestDiff) {
            lowestDiff = diff;
            vertex = *vp.first;
        }
    }


    // Check whether the best fit is satisfactory
    if ( (highestTTest > (ALPHA/2)) || // T-Test
         ( Utils::within
                 (
                 graph[vertex].mean,
                 stat.mean,
                 (Utils::largest(graph[vertex].mean, stat.mean) * 0.2)
                 ) // check if the means are within 20% of each other
         ))
        *success = true;
    else
        *success = false;

    return vertex;
}

void PowerStateGraph::updateOrInsertEdge(
        const Graph::vertex_descriptor& beforeVertex,
        const Graph::vertex_descriptor& afterVertex,
        const size_t sampleSinceLastSpike,
        const double spikeDelta
        )
{
    /* Add an edge from beforeVertex to afterVertex.
     * If an edge already exists then boost::add_edge will
     * return an edge_descriptor to that edge. */

    Graph::edge_descriptor edge;
    bool existingEdge;
    tie(edge, existingEdge) = boost::add_edge(beforeVertex, afterVertex, graph);

    if (existingEdge) { // then there was not an edge already in the graph
        graph[edge].delta    = Statistic<double>( spikeDelta );
        graph[edge].duration = Statistic<size_t>( sampleSinceLastSpike );
        graph[edge].count    = 1;
    } else { // there was an edge already in the graph so update that edge
        graph[edge].delta.update( spikeDelta );
        graph[edge].duration.update( sampleSinceLastSpike );
        graph[edge].count++;
    }

    totalCount++;
}

/**
 * @brief Update (or create) directional edges between power states (vertices)
 *        so the edges are consistent with the observed transitions between
 *        power states in @c sig.
 *
 * Basic strategy is to:
 * <ol>
 *  <li>retrieve gradient spikes from @c sig, extract the most salient
 *      and put the spikes into temporal order.</li>
 *  <li>Create stats for the data points between each spike. Use mostSimilarVertex()
 *      to determine which of the existing power state vertices this belongs to.</li>
 *  <li>Check to see if there's an existing edge representing the power state transition,
 *      if so then check the @c delta and @c duration stats for the edge and update if necessary.
 *      If no edge exists then create one with the necessary @c delta and @c duration,
 *      with a hard-coded stdev.</li>
 * </ol>
 *
 *  @deprecated just leaving this here to illustrate one of the strategies I tried.
 *  This has been superceded by update() (which used to be updateVertices) and updateOrInsertEdge()
 *
 */
void PowerStateGraph::updateEdges( const Signature& sig )
{
    Statistic<Sample_t> nextPowerState, prevPowerState;
    Graph::vertex_descriptor mostSimVertex, previousVertex;
    pair<Graph::edge_descriptor, bool> addedEdge;
    size_t previousIndex;

    // get the gradient spikes for the first signature
    list<Signature::Spike> spikes = sig.getGradientSpikesInOrder();

    // take just the top ten (whilst ordered by absolute value)
    if (spikes.size() > 10) {
        list<Signature::Spike>::iterator it = spikes.begin();
        advance( it, 10 );
        spikes.erase( it, spikes.end() );
    }

    // re-order by index (i.e. by time)
    spikes.sort( Signature::Spike::compareIndexAsc );

    for (list<Signature::Spike>::const_iterator it=spikes.begin(); it!=spikes.end(); it++) {
        cout << it->index << "\t" << it->delta << endl;
    }

    // calculate stats for the signature's values between start and first spike
    // as long as the first spike->index > 1
    previousVertex = offVertex;
    previousIndex  = 0;

    list<Signature::Spike>::iterator spike, prevSpike;
    prevSpike = spike = spikes.begin();
    advance(spike, 1);
    for (; spike!=spikes.end(); spike++ ) {

        nextPowerState = Statistic<Sample_t>( sig, prevSpike->index+1, prevSpike->index+10); // spike->index);

        bool foundSimilar;
        mostSimVertex = mostSimilarVertex( &foundSimilar, nextPowerState, 0.1 );

        if (foundSimilar && previousVertex!=mostSimVertex) {
            /* Add an edge from previousVertex to mostSimVertex.
             * If an edge already exists then boost::add_edge will
             * return an edge_descriptor to that edge. */
            addedEdge = boost::add_edge(previousVertex, mostSimVertex, graph);

            if (addedEdge.second) { // then there was not an edge already in the graph
                graph[addedEdge.first].delta =
                        Statistic<double>( prevSpike->delta );
                graph[addedEdge.first].duration =
                        Statistic<size_t>( prevSpike->index - previousIndex );
                graph[addedEdge.first].count = 1;
                totalCount++;
            } else { // there was an edge already in the graph so update that edge
                graph[addedEdge.first].delta.update( prevSpike->delta );
                graph[addedEdge.first].duration.update( prevSpike->index - previousIndex );
                graph[addedEdge.first].count++;
                totalCount++;
            }

            previousIndex  = prevSpike->index;
            previousVertex = mostSimVertex;
        }
        prevSpike = spike;
    }

    // add the last edge back to "off"
    addedEdge = boost::add_edge(previousVertex, offVertex, graph);
    if (addedEdge.second) { // then there was not an edge already in the graph
        graph[addedEdge.first].delta =
                Statistic<double>( prevSpike->delta );
        graph[addedEdge.first].duration =
                Statistic<size_t>( prevSpike->index - previousIndex );
        graph[addedEdge.first].count = 1;
        totalCount++;
    } else { // there was an edge already in the graph so update that edge
        graph[addedEdge.first].delta.update( prevSpike->delta );
        graph[addedEdge.first].duration.update( prevSpike->index - previousIndex );
        graph[addedEdge.first].count++;
        totalCount++;
    }

}

void PowerStateGraph::writeGraphViz(ostream& out)
{

    char * str = 0;
    size_t i = 0;

    // generate names for vertices
    const char* vertexName[ num_vertices(graph) ];
    PowerStateGraph::VertexIndexMap vertexIndex = boost::get(boost::vertex_index, graph);
    std::pair<PowerStateGraph::vertex_iter, PowerStateGraph::vertex_iter> vp;
    for (vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
        str = new char[30];
        sprintf(str, "min%d mean%d max%d sd%d",
                Utils::roundToNearestInt(graph[*vp.first].min),
                Utils::roundToNearestInt(graph[*vp.first].mean),
                Utils::roundToNearestInt(graph[*vp.first].max),
                Utils::roundToNearestInt(graph[*vp.first].stdev)
                );
        vertexName[i++] = str;
    }

    // generate names for edges
    const char* edgeName[ num_edges(graph) ];
    i=0;
    PowerStateGraph::EdgeIndexMap edgeIndex = boost::get(boost::edge_index, graph);
    std::pair<PowerStateGraph::edge_iter, PowerStateGraph::edge_iter> ep;
    for (ep = boost::edges(graph); ep.first != ep.second; ++ep.first) {
        str = new char[30];
/*        sprintf(str, "delta%d dur%d c%d",
                Utils::roundToNearestInt(graph[*ep.first].delta.mean ),
                Utils::roundToNearestInt(graph[*ep.first].duration.mean ),
                (int)graph[*ep.first].count
                );
                */

        edgeName[i++] = str;
    }

    write_graphviz(out, graph, boost::make_label_writer(vertexName), my_edge_writer(graph));

    // now delete the dynamically allocated strings
    for (i = 0; i<num_vertices(graph); i++) {
        delete [] vertexName[i];
    }
    for (i = 0; i<num_edges(graph); i++) {
        delete [] edgeName[i];
    }
}

std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg )
{
    PowerStateGraph::VertexIndexMap index = boost::get(boost::vertex_index, psg.graph);

    o << "vertices(graph) = " << std::endl;
    std::pair<PowerStateGraph::vertex_iter, PowerStateGraph::vertex_iter> vp;
    for (vp = boost::vertices(psg.graph); vp.first != vp.second; ++vp.first) {
        o << "vertex" << index[*vp.first] << " = {" << psg.graph[*vp.first] <<  "}";
        if ( *vp.first == psg.offVertex ) {
            o << " (offVertex)";
        }
        o << std::endl;
    }

    return o;
}
