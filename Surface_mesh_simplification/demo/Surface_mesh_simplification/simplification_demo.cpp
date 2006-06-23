// Copyright (c) 2002  Max Planck Institut fuer Informatik (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// 
//
// Author(s)     : Fernando Cacciola

//Borland, Microsoft and Intel compiler are excluded
#if defined(__BORLANDC__) || defined(_MSC_VER)

#include <iostream>
#include <fstream>

int main() {
  std::cout << "Geomview doesn't work on Windows, so no demo." << std::endl;
  return 0;
}
#else // not windows

#include <iostream>
#include <iomanip>

//#define VISUALIZE

#include <CGAL/Cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_BGL.h>
#include <CGAL/Polyhedron_extended_BGL.h>
#include <CGAL/Polyhedron_BGL_properties.h>
#include <CGAL/IO/Polyhedron_geomview_ostream.h>
#include <CGAL/IO/Polyhedron_iostream.h>

//#define CGAL_SURFACE_SIMPLIFICATION_ENABLE_TRACE 4

void Surface_simplification_external_trace( std::string s )
{
  std::cout << s << std::endl ;
} 

#include <CGAL/Surface_mesh_simplification_vertex_pair_collapse.h>

#include <CGAL/Surface_mesh_simplification/Policies/Construct_minimal_collapse_data.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Midpoint_vertex_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/LindstromTurk.h>
#include <CGAL/Surface_mesh_simplification/Policies/Count_stop_pred.h>

#include <iostream>
#include <fstream>

template <class Refs, class Traits>
struct My_vertex : public CGAL::HalfedgeDS_vertex_base<Refs,CGAL::Tag_true,typename Traits::Point_3> 
{
  typedef CGAL::HalfedgeDS_vertex_base<Refs,CGAL::Tag_true,typename Traits::Point_3> Base ;
 
  My_vertex() {} 
  My_vertex( typename Traits::Point_3 p ) : Base(p) {}
  
  int ID; 
} ;

template <class Refs, class Traits>
struct My_halfedge : public CGAL::HalfedgeDS_halfedge_base<Refs> 
{ 
  My_halfedge() {}
 
  int ID; 
};

template <class Refs, class Traits>
struct My_face : public CGAL::HalfedgeDS_face_base<Refs,CGAL::Tag_true,typename Traits::Plane_3>
{
  typedef CGAL::HalfedgeDS_face_base<Refs,CGAL::Tag_true,typename Traits::Plane_3> Base ;
  
  My_face() {}
  My_face( typename Traits::Plane_3 plane ) : Base(plane) {}
  
  int ID; 
};

struct My_items : public CGAL::Polyhedron_items_3 
{
    template < class Refs, class Traits>
    struct Vertex_wrapper {
        typedef My_vertex<Refs,Traits> Vertex;
    };
    template < class Refs, class Traits>
    struct Halfedge_wrapper {
        typedef My_halfedge<Refs,Traits>  Halfedge;
    };
    template < class Refs, class Traits>
    struct Face_wrapper {
        typedef My_face<Refs,Traits> Face;
    };
};

typedef CGAL::Cartesian<double>                              Kernel;
typedef Kernel::Vector_3                                     Vector;
typedef Kernel::Point_3                                      Point;
typedef CGAL::Polyhedron_3<Kernel,My_items>                  Polyhedron;

typedef Polyhedron::Vertex                                   Vertex;
typedef Polyhedron::Vertex_iterator                          Vertex_iterator;
typedef Polyhedron::Halfedge_handle                          Halfedge_handle;
typedef Polyhedron::Edge_iterator                            Edge_iterator;
typedef Polyhedron::Facet_iterator                           Facet_iterator;
typedef Polyhedron::Halfedge_around_vertex_const_circulator  HV_circulator;
typedef Polyhedron::Halfedge_around_facet_circulator         HF_circulator;

void create_center_vertex( Polyhedron& P, Facet_iterator f) {
    Vector vec( 0.0, 0.0, 0.0);
    std::size_t order = 0;
    HF_circulator h = f->facet_begin();
    do {
        vec = vec + ( h->vertex()->point() - CGAL::ORIGIN);
        ++ order;
    } while ( ++h != f->facet_begin());
    CGAL_assertion( order >= 3); // guaranteed by definition of polyhedron
    Point center =  CGAL::ORIGIN + (vec / order);
    Halfedge_handle new_center = P.create_center_vertex( f->halfedge());
    new_center->vertex()->point() = center;
}

struct Smooth_old_vertex {
    Point operator()( const Vertex& v) const {
        CGAL_precondition((CGAL::circulator_size( v.vertex_begin()) & 1) == 0);
        std::size_t degree = CGAL::circulator_size( v.vertex_begin()) / 2;
        double alpha = ( 4.0 - 2.0 * std::cos( 2.0 * CGAL_PI / degree)) / 9.0;
        Vector vec = (v.point() - CGAL::ORIGIN) * ( 1.0 - alpha);
        HV_circulator h = v.vertex_begin();
        do {
            vec = vec + ( h->opposite()->vertex()->point() - CGAL::ORIGIN) 
                       * alpha / degree;
            ++ h;
            CGAL_assertion( h != v.vertex_begin()); // even degree guaranteed
            ++ h;
        } while ( h != v.vertex_begin());
        return (CGAL::ORIGIN + vec);
    }
};

void flip_edge( Polyhedron& P, Halfedge_handle e) {
    Halfedge_handle h = e->next();
    P.join_facet( e);
    P.split_facet( h, h->next()->next());
}

void subdiv( Polyhedron& P) {
    if ( P.size_of_facets() == 0)
        return;
    // We use that new vertices/halfedges/facets are appended at the end.
    std::size_t nv = P.size_of_vertices();
    Vertex_iterator last_v = P.vertices_end();
    -- last_v;  // the last of the old vertices
    Edge_iterator last_e = P.edges_end();
    -- last_e;  // the last of the old edges
    Facet_iterator last_f = P.facets_end();
    -- last_f;  // the last of the old facets

    Facet_iterator f = P.facets_begin();    // create new center vertices
    do {
        create_center_vertex( P, f);
    } while ( f++ != last_f);

    std::vector<Point> pts;                    // smooth the old vertices
    pts.reserve( nv);  // get intermediate space for the new points
    ++ last_v; // make it the past-the-end position again
    std::transform( P.vertices_begin(), last_v, std::back_inserter( pts), 
                    Smooth_old_vertex());
    std::copy( pts.begin(), pts.end(), P.points_begin());

    Edge_iterator e = P.edges_begin();              // flip the old edges
    ++ last_e; // make it the past-the-end position again
    while ( e != last_e) {
        Halfedge_handle h = e;
        ++e; // careful, incr. before flip since flip destroys current edge
        flip_edge( P, h);
    };
    CGAL_postcondition( P.is_valid());
}

// This is here only to allow a breakpoint to be placed so I can trace back the problem.
void error_handler ( char const* what, char const* expr, char const* file, int line, char const* msg )
{
  std::cerr << "CGAL error: " << what << " violation!" << std::endl 
       << "Expr: " << expr << std::endl
       << "File: " << file << std::endl 
       << "Line: " << line << std::endl;
  if ( msg != 0)
      std::cerr << "Explanation:" << msg << std::endl;
}

using namespace CGAL::Triangulated_surface_mesh::Simplification ;

int Simplify_midpoint ( Polyhedron& aP, int aMax )
{
  typedef Minimal_collapse_data<Polyhedron> CollapseData ;
  
  Construct_minimal_collapse_data<Polyhedron> Construct_collapse_data ;
  Edge_length_cost         <CollapseData>     Get_cost ;
  Midpoint_vertex_placement<CollapseData>     Get_vertex_point ;
  Count_stop_condition     <CollapseData>     Should_stop(aMax);
      
  return vertex_pair_collapse(aP,Construct_collapse_data,(void*)0,Get_cost,Get_vertex_point,Should_stop);
}

int Simplify_LT ( Polyhedron& aP, int aMax )
{
  typedef LindstromTurk_collapse_data<Polyhedron> CollapseData ;
  
  Construct_LindstromTurk_collapse_data<Polyhedron>    Construct_collapse_data ;
  LindstromTurk_cost                   <Polyhedron>    Get_cost ;
  LindstromTurk_vertex_placement       <Polyhedron>    Get_vertex_point ;
  Count_stop_condition                 <CollapseData>  Should_stop(aMax);
      
  LindstromTurk_params lParams(1,1,1);
  
  return vertex_pair_collapse(aP,Construct_collapse_data,&lParams,Get_cost,Get_vertex_point,Should_stop);
}

void Simplify ( Polyhedron& aP, int aMax )
{
 std::cout << "Simplifying surface with " << (aP.size_of_halfedges()/2) << " edges..." << std::endl ;

#ifdef VISUALIZE
  CGAL::Geomview_stream gv;
  gv.set_bg_color(CGAL::BLACK);
  gv.set_face_color(CGAL::WHITE);
  gv.set_edge_color(CGAL::BLUE);
  gv.set_vertex_color(CGAL::RED);
#endif    

  std::cout << std::setprecision(19) ;

  int lVertexID = 0 ;
  for ( Polyhedron::Vertex_iterator vi = aP.vertices_begin(); vi != aP.vertices_end() ; ++ vi )
    vi->ID = lVertexID ++ ;    
    
  int lHalfedgeID = 0 ;
  for ( Polyhedron::Halfedge_iterator hi = aP.halfedges_begin(); hi != aP.halfedges_end() ; ++ hi )
    hi->ID = lHalfedgeID++ ;    
  
  int lFacetID = 0 ;
  for ( Polyhedron::Facet_iterator fi = aP.facets_begin(); fi != aP.facets_end() ; ++ fi )
    fi->ID = lFacetID ++ ;    


  int r = Simplify_LT(aP,aMax);
      
  std::cout << "Finished...\n"
            << r << " edges removed.\n"
            << aP.size_of_vertices() << " vertices.\n"
            << (aP.size_of_halfedges()/2) << " edges.\n"
            << aP.size_of_facets() << " triangles.\n" 
            << ( aP.is_valid() ? " valid" : " INVALID!!" )
            << std::endl  ;

#ifdef VISUALIZE
  gv << aP ;
  std::cout << "Press any key to finish..." << std::endl ;
  char k ;
  std::cin >> k ;
#endif    
}

int main( int argc, char** argv ) 
{
    CGAL::set_error_handler  (error_handler);
    CGAL::set_warning_handler(error_handler);
  
    Polyhedron lP; 
    
    char const* infile = argc > 1 ? argv[1] : "./data/tetra2.off" ; //"./data/Sample0.off" ;
    std::ifstream in(infile);
    if ( in )
    {
      in >> lP ;
      
      int lMax = argc > 2 ? std::atoi(argv[2]) : 1 ; // 1000
            
      Simplify(lP,lMax);
      
      char const* of = argc > 3 ? argv[3] : 0 ;
      std::string outfile = !of ? std::string(infile) + std::string(".out.off") : of ;
      
      std::ofstream out(outfile.c_str());
      if ( out )
      {
        out << lP ;
      }
      else
      {
        std::cerr << "Unable to open out file: " << outfile << std::endl ;
      }
    }
    else
    {
      std::cerr << "Input file not found: " << infile << std::endl ;
    }
      
  

    return 0;
}

#endif // not windows
// EOF //
