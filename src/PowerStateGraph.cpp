/*
 * PowerStateGraph.cpp
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#include "PowerStateGraph.h"
#include <iostream>
#include <list>

using namespace std;

PowerStateGraph::PowerStateGraph()
{
    using namespace boost;

    // add a vertex to represent "off"
    offVertex = add_vertex(graph);
    graph[offVertex] = Statistic<Sample_t>(0);

}

PowerStateGraph::~PowerStateGraph()
{
    // TODO Auto-generated destructor stub
}

void PowerStateGraph::updateVertices( const Signature& sig )
{
    Statistic<Sample_t> powerState;

    // get the gradient spikes for the first signature
    list<Signature::Spike> spikes = sig.getGradientSpikesInOrder();
    list<Signature::Spike>::iterator spike, prevSpike;

    // take just the top ten (whilst ordered by absolute value)
/*    if (spikes.size() > 10) {
        list<Signature::Spike>::iterator it = spikes.begin();
        advance( it, 10 );
        spikes.erase( it, spikes.end() );
    }
*/

    // re-order by index (i.e. by time)
    spikes.sort( Signature::Spike::compareIndexAsc );

    // calculate stats for the signature's values between start and first spike
    // as long as the first spike->index > 1
    spike = spikes.begin();
    if ( spike->index > 1 ) {
        updateOrInsertVertex( Statistic<Sample_t>( sig, 1, spike->index ), sig, 1, spike->index );
    }

    // go through each subsequent spike, updating an existing vertex or adding a new one
    prevSpike = spike;
    while ( ++spike != spikes.end() ) {
        updateOrInsertVertex( Statistic<Sample_t>( sig, prevSpike->index, spike->index ),
                sig, prevSpike->index, spike->index );

        prevSpike = spike;
    }
}

/**
 * @brief Attempts to find an existing vertex which is
 * statistically similar to @c stat.  If an existing similar
 * vertex is found then the vertex's stats are updated
 * with the new data points. If a similar vertex is not found,
 * and new vertex is inserted.
 */
void PowerStateGraph::updateOrInsertVertex(
        const Statistic<Sample_t>& stat, /**< stat to find / insert in graph vertices */
        const Signature& sig, /**< source of the raw data */
        const size_t start,   /**< start of data window */
        const size_t end      /**< end of data window */
    )
{
    if ( ( end - start ) < 5)
        return; // ignore little stretches of data

    bool foundSimilar; // return param for mostSimilarVertex()

    Graph::vertex_descriptor mostSimVertex = mostSimilarVertex( &foundSimilar, stat );

    if ( foundSimilar ) {
        graph[mostSimVertex].update( sig, start+1, end );
    } else {
        // then add a new vertex
        Graph::vertex_descriptor newVertex = add_vertex(graph);
        graph[newVertex] = Statistic<Sample_t>( sig, start+1, end );
    }

}

/**
 * @brief Find the statistically most similar vertex to @c stat.
 *
 * @return vertex descriptor of best fit.
 *         @c success is also used as a return parameter.
 */
PowerStateGraph::Graph::vertex_descriptor PowerStateGraph::mostSimilarVertex(
        bool * success, /**< return parameter.  Did we find a satisfactory match? */
        const Statistic<Sample_t>& stat /**< stat to find in graph vertices */
    )
{
    const double ALPHA = 0.00000000000000005; // significance level (what constitutes as a "satisfactory" match?)
    Graph::vertex_descriptor vertex=0;
    std::pair<vertex_iter, vertex_iter> vp;
    double tTest, highestTTest=0;

    // Find the best fit
    for (vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
        tTest = stat.tTest( graph[*vp.first] );
        if (tTest > highestTTest) {
            highestTTest = tTest;
            vertex = *vp.first;
        }
    }

    // Check whether the best fit is satisfactory
    if ( highestTTest > (ALPHA/2) )
        *success = true;
    else
        *success = false;

    return vertex;
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
 *  @todo should 'salience' also take into consideration duration?
 *
 */
void PowerStateGraph::updateEdges( const Signature& sig )
{
    Statistic<Sample_t> powerState;
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

    // calculate stats for the signature's values between start and first spike
    // as long as the first spike->index > 1
    previousVertex = offVertex;
    previousIndex  = 0;
    list<Signature::Spike>::iterator spike = spikes.begin();
    if ( spike->index > 1 ) {
        powerState = Statistic<Sample_t>( sig, 1, spike->index );
        bool foundSimilar;
        mostSimVertex = mostSimilarVertex( &foundSimilar, powerState );

        /* Add an edge from previousVertex to mostSimVertex.
         * If an edge already exists then boost::add_edge will
         * return an edge_descriptor to that edge. */
        addedEdge = boost::add_edge(previousVertex, mostSimVertex, graph);

        if (addedEdge.second) { // then there was not an edge already in the graph
            graph[addedEdge.first].delta =
                    Statistic<double>( spike->delta );
            graph[addedEdge.first].duration =
                    Statistic<size_t>( spike->index - previousIndex );
        } else { // there was an edge already in the graph so update that edge
            graph[addedEdge.first].delta.update( spike->delta );
            graph[addedEdge.first].duration.update( spike->index - previousIndex );
        }

    }




}

std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg )
{
    PowerStateGraph::IndexMap index = boost::get(boost::vertex_index, psg.graph);

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
