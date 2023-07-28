#include "HalfedgeMesh.hpp"
#include "Tuplet.hpp"

#include <vector>
#include <algorithm> // for sort()
#include <numeric>   // for iota()

using namespace std;

namespace Bem {

HalfedgeMesh::~HalfedgeMesh() {
    for(Halfedge* elm : trigs) {
        delete elm->next->next;
        delete elm->next;
        delete elm;
    }
    vpos.clear();
    verts.clear();
    edges.clear();
    trigs.clear();
}

HalfedgeMesh::HalfedgeMesh(Mesh const& other) 
    :HalfedgeMesh(generate_halfedges(other)) {
}

HalfedgeMesh::HalfedgeMesh(HalfedgeMesh const& other) {
    copy(other);
}
HalfedgeMesh& HalfedgeMesh::operator=(HalfedgeMesh const& other) {
    copy(other);
    return *this;
}

void HalfedgeMesh::copy(HalfedgeMesh const& other) {
    cout << "making copy." << endl;
    vpos = other.vpos;
    verts = vector<Halfedge*>(other.verts.size(),nullptr);
    edges = vector<Halfedge*>(other.edges.size(),nullptr);
    trigs = vector<Halfedge*>(other.trigs.size(),nullptr);
    for(Halfedge* elm : other.edges) {
        if(elm->twin == elm) { // if at boundary
            Halfedge* A = new Halfedge;

            A->vert = elm->vert;
            A->edge = elm->edge;
            A->trig = elm->trig;
            A->twin = A;

            verts[A->vert] = A;
            edges[A->edge] = A;
            trigs[A->trig] = A;
        } else {
            Halfedge* A = new Halfedge;
            Halfedge* B = new Halfedge;

            A->vert = elm->vert;
            A->edge = elm->edge;
            A->trig = elm->trig;
            A->twin = B;

            B->vert = elm->twin->vert;
            B->edge = elm->twin->edge;
            B->trig = elm->twin->trig;
            B->twin = A;

            verts[A->vert] = A;
            edges[A->edge] = A;
            trigs[A->trig] = A;

            verts[B->vert] = B;
            edges[B->edge] = B;
            trigs[B->trig] = B; 
        }
    }
    for(Halfedge* elm : other.trigs) {
        size_t trig_ind = elm->trig;
        Halfedge* A = edges[elm->edge];
        Halfedge* B = edges[elm->next->edge];
        Halfedge* C = edges[elm->next->next->edge];
        if(A->trig != trig_ind) A = A->twin;
        if(B->trig != trig_ind) B = B->twin;
        if(C->trig != trig_ind) C = C->twin;

        A->next = B;
        B->next = C;
        C->next = A;
    }
}

HalfedgeMesh generate_halfedges(Mesh const& mesh) {

    HalfedgeMesh result;
    
    // output arrays
    result.vpos = mesh.verts;
    vector<Halfedge*>& verts = result.verts;
    verts = vector<Halfedge*>(mesh.verts.size(),nullptr);
    vector<Halfedge*>& trigs = result.trigs;

    vector<Tuplet> edges;
    vector<Halfedge*> edge_halfs;

    for(size_t i(0);i<mesh.trigs.size();++i) {
        Triplet t(mesh.trigs[i]);

        Halfedge* A = new Halfedge;
        Halfedge* B = new Halfedge;
        Halfedge* C = new Halfedge;

        // here all relations between faces,vertices and halfedges are created

        A->next = B;
        A->vert = t.a;
        A->trig = i;

        B->next = C;
        B->vert = t.b;
        B->trig = i;

        C->next = A;
        C->vert = t.c;
        C->trig = i;

        trigs.push_back(A);

        verts[t.a] = A; // these are overwritten multiple times in this loop
        verts[t.b] = B;
        verts[t.c] = C;

        // the "glue" between the triangles is given by the edges, for which 
        // we have to generate a list using the Tuplet class

        edges.push_back(Tuplet(t.a,t.b));
        edges.push_back(Tuplet(t.b,t.c));
        edges.push_back(Tuplet(t.c,t.a));

        edge_halfs.push_back(A);
        edge_halfs.push_back(B);
        edge_halfs.push_back(C);
    }

    size_t K(edges.size());
    vector<size_t> edge_inds(K);
    iota(edge_inds.begin(),edge_inds.end(),0);
    sort(edge_inds.begin(),edge_inds.end(),[&edges](size_t a,size_t b) { return edges[a] < edges[b]; });

    size_t edge_ind(0);
    for(size_t i(0);i<K;++i) {

        if(i == K-1 or edges[edge_inds[i]]<edges[edge_inds[i+1]]) { 
            // if the edge was pushed back two times (from two triangles), the edge is
            // an interior edge. The present condition evaluates false, since the two 
            // indices are identical. In the other case, the edge was pushed back only
            // once and therefore we have a boundary edge, the case treated here:

            size_t ind = edge_inds[i];
            Halfedge* A = edge_halfs[ind];

            A->twin = A;
            A->edge = edge_ind;

            result.edges.push_back(A);

        } else {

            size_t ind1 = edge_inds[i];
            i++;
            size_t ind2 = edge_inds[i];

            Halfedge* A = edge_halfs[ind1];
            Halfedge* B = edge_halfs[ind2];

            // now we can add the final informations to the halfedges

            A->twin = B;
            B->twin = A;

            A->edge = edge_ind;
            B->edge = edge_ind;

            result.edges.push_back(A);
        }

        edge_ind++;

    }

    return result;
}

Mesh generate_mesh(HalfedgeMesh const& mesh) {
    Mesh result;
    result.verts = mesh.vpos;
    for(Halfedge* elm : mesh.trigs) {
        Triplet t(elm->vert,elm->next->vert,elm->next->next->vert);
        result.trigs.push_back(t);
    }
    return result;
}


} // namespace Bem