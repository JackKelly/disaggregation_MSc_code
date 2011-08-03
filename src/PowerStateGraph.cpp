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
    graph[offVertex] = Statistic<Sample_t>(0,0,0,0);

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

    // remove any spikes under 10 Watts
    spike = spikes.begin();
    while (spike != spikes.end()) {
        if (fabs(spike->delta) < 10) {
            spikes.erase( spike++ );
        } else {
            spike++;
        }
    }

    // re-order by index (i.e. by time)
    spikes.sort( Signature::Spike::compareIndexAsc );

//    for (list<Signature::Spike>::iterator it = spikes.begin(); it!=spikes.end(); it++) {
//        cout << it->index << "\t" << it->delta << "\t" << it->duration << endl;
//    }

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

void PowerStateGraph::updateOrInsertVertex(
        const Statistic<Sample_t>& stat,
        const Signature& sig,
        const size_t start,
        const size_t end
    )
{
    if ( ( end - start ) < 5)
        return; // ignore little stretches of data

    std::pair<vertex_iter, vertex_iter> vp;
    bool foundSimilar = false;

    for (vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
        if (stat.similar( graph[*vp.first], 0.00000000000000005 ) ) {
            foundSimilar = true;
            graph[*vp.first].update( sig, start+1, end );
            break;
        }
    }

    if ( ! foundSimilar ) {
        // then add a new vertex
        Graph::vertex_descriptor newVertex = add_vertex(graph);
        graph[newVertex] = Statistic<Sample_t>( sig, start+1, end );
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
