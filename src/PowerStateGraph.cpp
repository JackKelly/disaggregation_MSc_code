/*
 * PowerStateGraph.cpp
 *
 *  Created on: 3 Aug 2011
 *      Author: jack
 */

#include "PowerStateGraph.h"
#include "AggregateData.h"
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
    // uses Statistic's default constructor to make a stat with all-zeros
    offVertex = add_vertex(powerStateGraph);

}

PowerStateGraph::~PowerStateGraph()
{
    // TODO Auto-generated destructor stub
}

/**
 * Update or initialise Power State Graph.
 */
void PowerStateGraph::update(
        const Signature& sig,
        const bool verbose
        )
{
    Statistic<Sample_t> powerState;

    // get the gradient spikes for the signature
    list<Signature::Spike> spikes = sig.getGradientSpikesInOrder();
    list<Signature::Spike>::iterator spike;
    size_t indexOfLastAcceptedSpike = 0;
    PSGraph::vertex_descriptor afterVertex, prevAcceptedVertex=offVertex;

    // take just the top ten (whilst ordered by absolute value)
    if (spikes.size() > 10) {
        list<Signature::Spike>::iterator it = spikes.begin();
        advance( it, 10 );
        spikes.erase( it, spikes.end() );
    }

    // re-order by index (i.e. by time)
    spikes.sort( Signature::Spike::compareIndexAsc );

    // for each spike, locate the samples immediately before and immediately after the spike
    const size_t WINDOW = 8; // how far either side of the spike will we look?
    for (spike = spikes.begin(); spike!=spikes.end(); spike++) {

        if (verbose) {
            cout << "SPIKE: index=" << spike->index
                 << ", delta=" << spike->delta
                 << ", duration=" << spike->duration
                 << endl;
        }

        size_t start = ((spike->index > WINDOW) ?
                (spike->index - WINDOW) : 0 );

        size_t end = spike->index + WINDOW + spike->duration + 1;
        if (end > sig.getSize())
            end = sig.getSize();

        Statistic<Sample_t> before(sig, start, spike->index);
        Statistic<Sample_t> after(sig, (spike->index + spike->duration + 1), end);

        if (verbose) {
            cout << "Before=" << before << endl
                 << "After =" << after  << endl;
        }

        if (rejectSpike(before, after) ||
            start < indexOfLastAcceptedSpike) {  // check the spikes aren't too close

            if (verbose) cout << " REJECT";

        } else {

            if (verbose) cout << " KEEP";

            afterVertex =
                    updateOrInsertVertex(  after, sig, (spike->index + spike->duration + 1), end);

            if (prevAcceptedVertex != afterVertex) {
                updateOrInsertEdge( prevAcceptedVertex, afterVertex,
                        (spike->index - indexOfLastAcceptedSpike), spike->delta );
            }

            prevAcceptedVertex = afterVertex;
            indexOfLastAcceptedSpike = spike->index;
        }
        if (verbose) cout << endl;

        if (verbose) {
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
}

const bool PowerStateGraph::rejectSpike(
        const Statistic<Sample_t>& before,
        const Statistic<Sample_t>& after,
        const bool verbose
        ) const
{
    // if either 'before' or 'after' has a stdev greater than
    // its mean then reject
    if ( (before.stdev > (before.mean)) ||
         ( after.stdev > ( after.mean))   ) {
        if (verbose) cout << "stdev too big";
        return true;
    }


    if (Utils::within(before.mean, after.mean, Utils::largest(before.stdev, after.stdev)*2) ||
        Utils::within(before.mean, after.mean, Utils::largest(before.mean, after.mean)*0.2)     ) {
        if (verbose) cout << "means too close";
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
PowerStateGraph::PSGraph::vertex_descriptor PowerStateGraph::updateOrInsertVertex(
        const Statistic<Sample_t>& stat, /**< stat to find or insert in graph vertices */
        const Signature& sig, /**< source of the raw data */
        const size_t start,   /**< start of data window */
        const size_t end,      /**< end of data window */
        const bool verbose  /**< cout debugging messages? */
    )
{

    bool foundSimilar; // return param for mostSimilarVertex()

    PSGraph::vertex_descriptor vertex = mostSimilarVertex( &foundSimilar, stat );

    if (verbose)
        cout << " mostSimlar = " << powerStateGraph[vertex] << endl;

    if ( foundSimilar ) {
        if (vertex != offVertex) {
            if (verbose) {
                cout << endl
                        << "Updating existing vertex: " << endl
                        << "    start=" << start << ", end=" << end << endl
                        << "    existing vertex = " << powerStateGraph[vertex] << endl
                        << "    new stat        = " << stat << endl;
            }
            powerStateGraph[vertex].update( sig, start, end );
            if (verbose)
                cout << "    merged          = " << powerStateGraph[vertex] << endl;
        }
    } else {
        // then add a new vertex
        vertex = add_vertex(powerStateGraph);
        powerStateGraph[vertex] = Statistic<Sample_t>( sig, start, end );
        if (verbose) {
            cout << endl
                    << "Adding new vertex: " << endl
                    << "    start=" << start << ", end=" << end << endl
                    << "    vertex = " << powerStateGraph[vertex] << endl
                    << "    stat   = " << stat << endl;
        }
    }
    return vertex;

}

/**
 * @brief Find the statistically most similar vertex to @c stat.
 *
 * @return vertex descriptor of best fit.
 *         @c success is also used as a return parameter.
 */
PowerStateGraph::PSGraph::vertex_descriptor PowerStateGraph::mostSimilarVertex(
        bool * success, /**< return parameter.  Did we find a satisfactory match? */
        const Statistic<Sample_t>& stat, /**< stat to find in graph vertices */
        const double ALPHA /**< significance level (what constitutes as a "satisfactory" match?) */
    ) const
{
    PSGraph::vertex_descriptor vertex=0;
    std::pair<PSG_vertex_iter, PSG_vertex_iter> vp;
    double highestTTest=0, diff, lowestDiff = std::numeric_limits<double>::max();

    // Find the best fit
    for (vp = boost::vertices(powerStateGraph); vp.first != vp.second; ++vp.first) {
        // t test
/*      tTest = stat.tTest( graph[*vp.first] );
        cout << "tTest=" << tTest << "  " << graph[*vp.first] << endl;
        if (tTest > highestTTest) {
            highestTTest = tTest;
            vertex = *vp.first;
        }
*/

        // Now compare means.  In the vast majority of the cases, comparing
        // means will produce the same best match as the tTest.
        diff = fabs( powerStateGraph[*vp.first].mean - stat.mean );
        if (diff < lowestDiff) {
            lowestDiff = diff;
            vertex = *vp.first;
        }
    }


    // Check whether the best fit is satisfactory
    if ( (highestTTest > (ALPHA/2)) || // T-Test
         ( Utils::within
                 (
                 powerStateGraph[vertex].mean,
                 stat.mean,
                 (Utils::largest(powerStateGraph[vertex].mean, stat.mean) * 0.2)
                 ) // check if the means are within 20% of each other
         ))
        *success = true;
    else
        *success = false;

    return vertex;
}

void PowerStateGraph::updateOrInsertEdge(
        const PSGraph::vertex_descriptor& beforeVertex,
        const PSGraph::vertex_descriptor& afterVertex,
        const size_t sampleSinceLastSpike,
        const double spikeDelta
        )
{
    /* Add an edge from beforeVertex to afterVertex.
     * If an edge already exists then boost::add_edge will
     * return an edge_descriptor to that edge. */

    PSGraph::edge_descriptor edge;
    bool existingEdge;
    tie(edge, existingEdge) = boost::add_edge(beforeVertex, afterVertex, powerStateGraph);

    if (existingEdge) { // then there was not an edge already in the graph
        powerStateGraph[edge].delta    = Statistic<double>( spikeDelta );
        powerStateGraph[edge].duration = Statistic<size_t>( sampleSinceLastSpike );
        powerStateGraph[edge].count    = 1;
    } else { // there was an edge already in the graph so update that edge
        powerStateGraph[edge].delta.update( spikeDelta );
        powerStateGraph[edge].duration.update( sampleSinceLastSpike );
        powerStateGraph[edge].count++;
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
    PSGraph::vertex_descriptor mostSimVertex, previousVertex;
    pair<PSGraph::edge_descriptor, bool> addedEdge;
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

    // Useful for seeing what's going on...
/*    for (list<Signature::Spike>::const_iterator it=spikes.begin(); it!=spikes.end(); it++) {
        cout << it->index << "\t" << it->delta << endl;
    }
*/
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
            addedEdge = boost::add_edge(previousVertex, mostSimVertex, powerStateGraph);

            if (addedEdge.second) { // then there was not an edge already in the graph
                powerStateGraph[addedEdge.first].delta =
                        Statistic<double>( prevSpike->delta );
                powerStateGraph[addedEdge.first].duration =
                        Statistic<size_t>( prevSpike->index - previousIndex );
                powerStateGraph[addedEdge.first].count = 1;
                totalCount++;
            } else { // there was an edge already in the graph so update that edge
                powerStateGraph[addedEdge.first].delta.update( prevSpike->delta );
                powerStateGraph[addedEdge.first].duration.update( prevSpike->index - previousIndex );
                powerStateGraph[addedEdge.first].count++;
                totalCount++;
            }

            previousIndex  = prevSpike->index;
            previousVertex = mostSimVertex;
        }
        prevSpike = spike;
    }

    // add the last edge back to "off"
    addedEdge = boost::add_edge(previousVertex, offVertex, powerStateGraph);
    if (addedEdge.second) { // then there was not an edge already in the graph
        powerStateGraph[addedEdge.first].delta =
                Statistic<double>( prevSpike->delta );
        powerStateGraph[addedEdge.first].duration =
                Statistic<size_t>( prevSpike->index - previousIndex );
        powerStateGraph[addedEdge.first].count = 1;
        totalCount++;
    } else { // there was an edge already in the graph so update that edge
        powerStateGraph[addedEdge.first].delta.update( prevSpike->delta );
        powerStateGraph[addedEdge.first].duration.update( prevSpike->index - previousIndex );
        powerStateGraph[addedEdge.first].count++;
        totalCount++;
    }

}

void PowerStateGraph::writeGraphViz(ostream& out)
{

    char * str = 0;
    size_t i = 0;

    // generate names for vertices
    const char* vertexName[ num_vertices(powerStateGraph) ];
    PowerStateGraph::PSG_vertex_index_map vertexIndex = boost::get(boost::vertex_index, powerStateGraph);
    std::pair<PowerStateGraph::PSG_vertex_iter, PowerStateGraph::PSG_vertex_iter> vp;
    for (vp = boost::vertices(powerStateGraph); vp.first != vp.second; ++vp.first) {
        str = new char[30];
        sprintf(str, "min%d mean%d max%d sd%d",
                Utils::roundToNearestInt(powerStateGraph[*vp.first].min),
                Utils::roundToNearestInt(powerStateGraph[*vp.first].mean),
                Utils::roundToNearestInt(powerStateGraph[*vp.first].max),
                Utils::roundToNearestInt(powerStateGraph[*vp.first].stdev)
                );
        vertexName[i++] = str;
    }

    // generate names for edges
    const char* edgeName[ num_edges(powerStateGraph) ];
    i=0;
    PowerStateGraph::PSG_edge_index_map edgeIndex = boost::get(boost::edge_index, powerStateGraph);
    std::pair<PowerStateGraph::PSG_edge_iter, PowerStateGraph::PSG_edge_iter> ep;
    for (ep = boost::edges(powerStateGraph); ep.first != ep.second; ++ep.first) {
        str = new char[30];
/*        sprintf(str, "delta%d dur%d c%d",
                Utils::roundToNearestInt(graph[*ep.first].delta.mean ),
                Utils::roundToNearestInt(graph[*ep.first].duration.mean ),
                (int)graph[*ep.first].count
                );
                */

        edgeName[i++] = str;
    }

    write_graphviz(out, powerStateGraph, boost::make_label_writer(vertexName), PSG_edge_writer(powerStateGraph));

    // now delete the dynamically allocated strings
    for (i = 0; i<num_vertices(powerStateGraph); i++) {
        delete [] vertexName[i];
    }
    for (i = 0; i<num_edges(powerStateGraph); i++) {
        delete [] edgeName[i];
    }
}

/**
 *
 * Each candidate solution is represented as a
 * directed acyclic graph (DAG).  Each vertex
 * on this tree is a spike found in the aggregate data.
 * The edge weights are the mean of the probability density
 * functions for the spike size and the timing.  If an
 * edge described in PowerStateGraph (from training) cannot be found
 * in the aggregate data then this candidate is discarded.  If
 * we successfully get from vertex0 back to vertex0 then the
 * shortest path through the tree is calculated and saved.
 *
 * <ol>
 * <li>Retrieve from @c graph edge @c e which connects @c vertex0 (offVertex)
 *     to vertex1.  Look through the AggregateData
 *     searching for any spike within a certain number of
 *     standard deviations of @c e.delta.</li>
 * <li>When a spike is found, start a DAG structure and
 *     start looking for the subsequent edges learnt during training.</li>
 * <li>Look for the delta corresponding to each out edge from current vertex.
 *     Store the UNIX timestamp of each candidate.  </li>
 * </ol>
 *
 * @return a list of UNIX times when the device starts
 */
const list<PowerStateGraph::DisaggregatedStruct> PowerStateGraph::getStartTimes(
        const AggregateData& aggData, /**< A populated array of AggregateData */
        const bool verbose
        ) const
{
    list<DisaggregatedStruct> disaggregateList;
    DisaggregatedStruct candidateDisaggStruct;

    // search through aggregateData for the delta corresponding
    // to the first edge from vertex0.
    PSG_out_edge_iter out_i, out_end;
    tie(out_i, out_end) = out_edges(offVertex, powerStateGraph);
    PowerStateEdge firstEdgeStats = powerStateGraph[*out_i];

    list<AggregateData::FoundSpike> foundSpikes = aggData.findSpike(firstEdgeStats.delta);

    for (list<AggregateData::FoundSpike>::const_iterator spike=foundSpikes.begin();
            spike!=foundSpikes.end();
            spike++) {

        candidateDisaggStruct = traceToEnd( *spike );

        if ( candidateDisaggStruct.confidence != -1 ) {
            disaggregateList.push_back( candidateDisaggStruct );
        }
    }

    return disaggregateList;
}

/**
 *
 * @return DisaggregatedStruct.confidence will be set to -1 if
 * this looks like it's not a good candidate match.
 */
const PowerStateGraph::DisaggregatedStruct PowerStateGraph::traceToEnd(
        const AggregateData::FoundSpike& spike
        ) const
{
    DisagGraph dag;

    // make the first vertex to represent "off"
    DisagGraph::vertex_descriptor dagOffVertex = add_vertex(dag);


    // add a vertex to represent the first true power state
    DisagGraph::vertex_descriptor firstVertex = add_vertex(dag);

    // retrieve info for firstVertex and for edge between dagOffVertex and firstVertex
    PSG_out_edge_iter out_i, out_end;
    PSG_vertex_iter v_i, v_end;
    tie(out_i, out_end) = out_edges(offVertex, powerStateGraph);
    PowerStateEdge firstEdgeStats = powerStateGraph[*out_i];
    tie(v_i, v_end) = vertices(powerStateGraph);
    v_i++;

    dag[firstVertex] = DisagVertex(
            firstEdgeStats.duration.mean,
            powerStateGraph[*v_i].mean,
            *v_i
            );

    // add an edge
    DisagGraph::edge_descriptor edge;
    bool existingEdge;
    tie(edge, existingEdge) = add_edge(dagOffVertex, firstVertex, spike.pdf, dag);

    write_graphviz(cout, dag, Disag_vertex_writer(dag));

}

std::ostream& operator<<( std::ostream& o, const PowerStateGraph& psg )
{
    PowerStateGraph::PSG_vertex_index_map index = boost::get(boost::vertex_index, psg.powerStateGraph);

    o << "vertices(graph) = " << std::endl;
    std::pair<PowerStateGraph::PSG_vertex_iter, PowerStateGraph::PSG_vertex_iter> vp;
    for (vp = boost::vertices(psg.powerStateGraph); vp.first != vp.second; ++vp.first) {
        o << "vertex" << index[*vp.first] << " = {" << psg.powerStateGraph[*vp.first] <<  "}";
        if ( *vp.first == psg.offVertex ) {
            o << " (offVertex)";
        }
        o << std::endl;
    }

    return o;
}
