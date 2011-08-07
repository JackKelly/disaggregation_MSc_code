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
#include <boost/math/distributions/normal.hpp>

using namespace std;

PowerStateGraph::PowerStateGraph()
: totalCount(0), aggData(0)
{
    using namespace boost;

    // add a vertex to represent "off"
    // uses Statistic's default constructor to make a Statistic with all-zeros
    offVertex = add_vertex(powerStateGraph);

}

PowerStateGraph::~PowerStateGraph()
{
    // TODO Auto-generated destructor stub
}

void PowerStateGraph::addItemToEdgeHistory(
        const PSGraph::edge_descriptor& edge
        )
{
    if (edgeHistory.size() >= EDGE_HISTORY_SIZE)
        edgeHistory.erase( edgeHistory.begin() );

    edgeHistory.push_back(edge);
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

    edgeHistory.clear();

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

        // calculate the start index for the window of signature data we'll examine
        size_t start = ((spike->index > WINDOW) ?
                (spike->index - WINDOW) : 0 );

        // calculate the end index
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
            // We're keeping this spike.

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

/**
 * @brief Update or Insert a new edge into powerStateGraph.
 *
 * - If an edge already exists between beforeVertex and afterVertex then
 *     - check all out-edges from beforeVertex
 *     - if any out-edge has the same history as the current history then
 *         - update that edge's statistics
 *     - else
 *         - create a new edge.
 * - else
 *     - create a new edge
 */
void PowerStateGraph::updateOrInsertEdge(
        const PSGraph::vertex_descriptor& beforeVertex,
        const PSGraph::vertex_descriptor& afterVertex,
        const size_t sampleSinceLastSpike,
        const double spikeDelta
        )
{
    totalCount++;

    cout << "-------------------" << endl;

    PSGraph::edge_descriptor existingEdge, newEdge;
    bool edgeExistsAlready;
    tie(existingEdge, edgeExistsAlready) = boost::edge(beforeVertex, afterVertex, powerStateGraph);

    if ( edgeExistsAlready )
        cout << "edgeExistsAlready" << endl;
    else
        cout << "edge does not Exist Already" << endl;

    if ( edgeExistsAlready ) {

        for (list<PSGraph::edge_descriptor>::iterator edge=edgeHistory.begin(); edge!=edgeHistory.end(); edge++) {
            cout << "                              edgeHistory = " << *edge << endl;
        }
        for (list<PSGraph::edge_descriptor>::iterator edge=powerStateGraph[existingEdge].edgeHistory.begin();
                edge!=powerStateGraph[existingEdge].edgeHistory.end(); edge++) {
            cout << "powerStateGraph[existingEdge].edgeHistory = " << *edge << endl;
        }

        cout << endl;

        // check if any of the out edges from beforeVertex have the same
        // history as our current history... if so, update that edge.
        PSGraph::out_edge_iterator out_e_i, out_e_end;
        tie(out_e_i, out_e_end) = out_edges(beforeVertex, powerStateGraph);
        for (; out_e_i != out_e_end; out_e_i++) {
            // check if the edge has the same history as out current history
            if ( edgeListsAreEqual(powerStateGraph[*out_e_i].edgeHistory, edgeHistory) ) {
                cout << "edge histories the same. merging with" << *out_e_i << powerStateGraph[*out_e_i].delta << endl;

                // update existing edge's stats
                powerStateGraph[*out_e_i].delta.update( spikeDelta );
                powerStateGraph[*out_e_i].duration.update( sampleSinceLastSpike );
                powerStateGraph[*out_e_i].count++;

                addItemToEdgeHistory( *out_e_i );

                return;
            }
        }
    }

    // if we get to here then we know that either:
    //    an edge doesn't already exist between these 2 vertices
    // or
    //    an edge does exist but it doesn't share our history.
    // either way, we need to add a new edge:

    tie(newEdge, edgeExistsAlready) = boost::add_edge(beforeVertex, afterVertex, powerStateGraph);
    powerStateGraph[newEdge].delta    = Statistic<double>( spikeDelta );
    powerStateGraph[newEdge].duration = Statistic<size_t>( sampleSinceLastSpike );
    powerStateGraph[newEdge].count    = 1;
    powerStateGraph[newEdge].edgeHistory = edgeHistory;

    cout << "adding new edge" << newEdge << " " << powerStateGraph[newEdge].delta << endl;

    addItemToEdgeHistory( newEdge );
}

const bool PowerStateGraph::edgeListsAreEqual(
        const list< PSGraph::edge_descriptor >& a,
        const list< PSGraph::edge_descriptor >& b
        ) const
{
    cout << "************" << endl;
    cout << "comparing lists:" << endl;

    for (list< PSGraph::edge_descriptor >::const_iterator i=a.begin(); i!=a.end(); i++) {
        cout << *i << endl;
    }
    cout << "----" << endl;
    for (list< PSGraph::edge_descriptor >::const_iterator i=b.begin(); i!=b.end(); i++) {
        cout << *i << endl;
    }

    if (a.size() != b.size()) {
        cout << "sizes not equal" << endl;
        return false;
    }

    cout << "************" << endl;

    list<PSGraph::edge_descriptor>::const_iterator a_i, b_i;

    a_i = a.begin();
    b_i = b.begin();

    while (a_i != a.end() && b_i != b.end()) {

        cout << "*a_i=" << *a_i << " *b_i=" << *b_i << endl;

        if ( source(*a_i, powerStateGraph) != source(*b_i, powerStateGraph) &&
             target(*a_i, powerStateGraph) != target(*b_i, powerStateGraph)   ) {

            cout << "edge list are not equal" << endl << endl;
            return false;
        }

        a_i++;
        b_i++;
    }

    return true;
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
 *  This has been superseded by update() (which used to be updateVertices) and updateOrInsertEdge()
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
    write_graphviz(
            out,
            powerStateGraph,
            PSG_vertex_writer(powerStateGraph),
            PSG_edge_writer(powerStateGraph)
            );
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
const list<PowerStateGraph::DisagDataItem> PowerStateGraph::getStartTimes(
        const AggregateData& aggregateData, /**< A populated array of AggregateData */
        const bool verbose
        )
{
    list<DisagDataItem> disagList; // what we return

    // Store a pointer to aggregateData for use later.
    aggData = &aggregateData;

    DisagDataItem candidateDisagDataItem;

    // search through aggregateData for the delta corresponding
    // to the first edge from vertex0.
    PSG_out_edge_iter out_i, out_end;
    tie(out_i, out_end) = out_edges(offVertex, powerStateGraph);
    PowerStateEdge firstEdgeStats = powerStateGraph[*out_i];

    list<AggregateData::FoundSpike> foundSpikes
    = aggregateData.findSpike(firstEdgeStats.delta);

    // for each spike which might possibly be the start of
    // the device signature in the aggregate reading,
    // attempt to find all the subsequent state changes.
    for (list<AggregateData::FoundSpike>::const_iterator spike=foundSpikes.begin();
            spike!=foundSpikes.end();
            spike++) {

        candidateDisagDataItem = initTraceToEnd(
                *spike,
                ( spike->timestamp - firstEdgeStats.duration.mean ) // time the device probably started
                );

        if ( candidateDisagDataItem.confidence != -1 ) {
            disagList.push_back( candidateDisagDataItem );
        }

        /**
         * @todo if two or more candidateDisagDataItems overlap then
         * remove all but the one with the highest confidence.
         */
    }

    return disagList;
}

/**
 *
 * @return DisaggregatedStruct.confidence will be set to -1 if
 * this looks like it's not a good candidate match.
 */
const PowerStateGraph::DisagDataItem PowerStateGraph::initTraceToEnd(
        const AggregateData::FoundSpike& spike,
        const size_t deviceStart /**< The possible time the device started. */
        ) const
{
    DisagTree disagTree;

    // make the first vertex (which represents "off")
    DisagTree::vertex_descriptor disagOffVertex = add_vertex(disagTree);
    disagTree[disagOffVertex].timestamp = deviceStart;
    disagTree[disagOffVertex].meanPower = 0; // this is "off"
    disagTree[disagOffVertex].psgVertex = offVertex;

    // add a vertex to represent the first true power state
    DisagTree::vertex_descriptor firstVertex = add_vertex(disagTree);

    // retrieve info for firstVertex and for edge between disagOffVertex and firstVertex
    PSG_out_edge_iter out_i, out_end;
    PSG_vertex_iter v_i, v_end;
    tie(out_i, out_end) = out_edges(offVertex, powerStateGraph);
    PowerStateEdge firstEdgeStats = powerStateGraph[*out_i];
    tie(v_i, v_end) = vertices(powerStateGraph);
    v_i++;

    disagTree[firstVertex].timestamp = spike.timestamp;
    disagTree[firstVertex].meanPower = powerStateGraph[*v_i].mean;
    disagTree[firstVertex].psgVertex = *v_i;
    disagTree[firstVertex].psgEdge   = *out_i;
    disagTree[firstVertex].edgeHistory.push_back(*out_i);

    // add an edge between disagOffVertex and firstVertex
    DisagTree::edge_descriptor edge;
    bool existingEdge;
    tie(edge, existingEdge) = add_edge(disagOffVertex, firstVertex, spike.pdf, disagTree);

    // now recursively trace from this edge to the end
    traceToEnd( &disagTree, firstVertex );

    write_graphviz(cout, disagTree,
            Disag_vertex_writer(disagTree), Disag_edge_writer(disagTree));

    // find route through the tree with highest average edge probabilities
    DisagDataItem disagDataItem;
    disagDataItem.timestamp  = deviceStart;
    disagDataItem.confidence = 0;
    disagDataItem.energy     = 0;
    disagDataItem.duration   = 0;

    EnergyConfidence ec =
            findBestPathThroughDisagTree( &disagDataItem, disagTree, disagOffVertex );

    cout << "ec.hops = " << ec.hops << " ec.confidence = " << ec.confidence << endl;

    return disagDataItem;

}

/**
 * @brief Trace the tree downwards from @c vertex recursively finding
 * the path with the highest average confidence and the corresponding
 * values for @c DisagDataItem.
 *
 * Traces the tree downwards.
 */
PowerStateGraph::EnergyConfidence PowerStateGraph::findBestPathThroughDisagTree(
        DisagDataItem * disagDataItem_p,
        const DisagTree& disagTree,
        const DisagTree::vertex_descriptor vertex
        ) const
{
    EnergyConfidence ec, ecDownstream; // used to locally keep track of this branch.

    // iterate through each out-edge
    Disag_out_edge_iter out_e_i, out_e_end;
    tie(out_e_i, out_e_end) = out_edges(vertex, disagTree);
    for (; out_e_i!=out_e_end; out_e_i++) {

        DisagTree::vertex_descriptor downstreamVertex = target(*out_e_i, disagTree);

        // check if this edge leads to a finish node
        if ( disagTree[ downstreamVertex ].meanPower == 0 ) {
            cout << *out_e_i << " takes us to a finish node" << endl;
            ec.hitDeadEnd = false;
            ec.confidence = disagTree[ *out_e_i ];
            ec.energy     = 0;
            ec.hops       = 1;
            return ec;
        } else {
            ecDownstream = findBestPathThroughDisagTree(
                    disagDataItem_p,
                    disagTree,
                    downstreamVertex );

            if ( ! ecDownstream.hitDeadEnd) {
                ec.hitDeadEnd = false;
                ec.hops       = ecDownstream.hops + 1;
                ec.confidence = ( disagTree[ *out_e_i ] +
                        ( ecDownstream.confidence / ecDownstream.hops )) / ec.hops ;
                return ec;
            }
        }
    }

    ec.hitDeadEnd = true;
    return ec;
}


/**
 * Trace from startVertex to the off state in PSGraph
 */
void PowerStateGraph::traceToEnd(
        DisagTree * disagGraph_p, /**< input and output parameter */
        const DisagTree::vertex_descriptor& startVertex
        ) const
{
    const double STDEV_MULT = 5; // higher = more permissive. (time)

    list<AggregateData::FoundSpike> foundSpikes;

    // A handy reference to make the code more readable
    DisagTree& disagGraph = *disagGraph_p;

    // base case
    if ( disagGraph[startVertex].psgVertex == offVertex ) {
        return;
    }

    // For each out-edge from startVertex.psgVertex, retrieve a list of
    // spikes which match and create a new vertex for each match.
    PSG_out_edge_iter psg_out_i, psg_out_end;
    tie(psg_out_i, psg_out_end) =
            out_edges(disagGraph[startVertex].psgVertex, powerStateGraph);

    for (; psg_out_i!=psg_out_end; psg_out_i++ ) {

        std::list< PSGraph::edge_descriptor > eHistory = getEdgeHistoryForVertex(disagGraph, startVertex);

        if ( ! edgeListsAreEqual(
//                eHistory,
                disagGraph[startVertex].edgeHistory,
                powerStateGraph[*psg_out_i].edgeHistory ) ) {
            cout << "edge histories not equal" << endl;
            continue;
        }

        //***************************************//
        // Calculate beginning of search window  //

        size_t begOfSearchWindow = disagGraph[startVertex].timestamp
                + powerStateGraph[*psg_out_i].duration.mean -
                (powerStateGraph[*psg_out_i].duration.stdev*STDEV_MULT);
        // ensure we're not looking backwards in time
        if (begOfSearchWindow < disagGraph[startVertex].timestamp)
            begOfSearchWindow = disagGraph[startVertex].timestamp;


        //***************************************//
        //    Calculate end of search window     //

        size_t endOfSearchWindow = disagGraph[startVertex].timestamp +
                powerStateGraph[*psg_out_i].duration.mean;

        // check that we're not looking past the end of aggData
        if (endOfSearchWindow > (*aggData)[ (*aggData).getSize() - 1 ].timestamp )
            continue;

        // add on stdev*STDEV_MULT
        endOfSearchWindow += powerStateGraph[*psg_out_i].duration.stdev*STDEV_MULT;


        //************************************************************//
        // get a list of candidate spikes matching this PSG out edge  //

        foundSpikes = (*aggData).findSpike(
                powerStateGraph[*psg_out_i].delta,  // spike stats
                begOfSearchWindow,
                endOfSearchWindow,
                9.5
                );


        //***************************************************************//
        // for each candidate spike, create a new vertex in disagGraph   //
        // and recursively trace this to the end                         //

        for (list<AggregateData::FoundSpike>::const_iterator spike=foundSpikes.begin();
                spike!=foundSpikes.end();
                spike++) {

            // calculate probability density function for the time at which the spike was found
            boost::math::normal dist(
                    powerStateGraph[*psg_out_i].duration.mean,
                    powerStateGraph[*psg_out_i].duration.stdev
                    );
            double pdf_for_time = boost::math::pdf(
                    dist,
                    spike->timestamp - disagGraph[startVertex].timestamp
                    );

            // create an average probability
            double avPdf = (pdf_for_time + spike->pdf) / 2;

            /** @todo experiment with weighting the avPdf in favour of the spike->pdf. */

            // check avPdf isn't too low
            if (avPdf < 0.00097)
                continue;

            // create new vertex
            DisagTree::vertex_descriptor newVertex=add_vertex(disagGraph);

            // create new edge
            DisagTree::edge_descriptor newEdge;
            bool existingEdge;
            tie(newEdge, existingEdge) =
                    add_edge(startVertex, newVertex, avPdf, disagGraph);


            // add details to newVertex
            disagGraph[newVertex].timestamp = spike->timestamp;
            // get vertex that *pst_out_i points to
            disagGraph[newVertex].psgVertex = target(*psg_out_i, powerStateGraph);
            disagGraph[newVertex].psgEdge   = *psg_out_i;
            disagGraph[newVertex].meanPower =
                    powerStateGraph[disagGraph[newVertex].psgVertex].mean;
            disagGraph[newVertex].edgeHistory = disagGraph[startVertex].edgeHistory;
            disagGraph[newVertex].edgeHistory.push_back(*psg_out_i);
            if (disagGraph[newVertex].edgeHistory.size() > EDGE_HISTORY_SIZE) {
                disagGraph[newVertex].edgeHistory.erase( disagGraph[newVertex].edgeHistory.begin() );
            }


            // recursively trace to end.
            traceToEnd(disagGraph_p, newVertex);
        }
    }
}

/**
 * Trace the disagGraph backwards
 */
list< PowerStateGraph::PSGraph::edge_descriptor > PowerStateGraph::getEdgeHistoryForVertex(
        const DisagTree& disagGraph,
        const DisagTree::vertex_descriptor& startVertex
        ) const
{
    list< PSGraph::edge_descriptor > eHistory;
    DisagTree::vertex_descriptor disagVertex = startVertex;

    DisagTree::in_edge_iterator in_e_i, in_e_end;

    while (disagGraph[disagVertex].psgVertex != offVertex && eHistory.size() < EDGE_HISTORY_SIZE) {
        eHistory.push_front( disagGraph[disagVertex].psgEdge );
        tie(in_e_i, in_e_end) = in_edges(disagVertex, disagGraph);
        disagVertex = source( *in_e_i, disagGraph ); // the vertex upstream from in_e_i
    }

    return eHistory;
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
