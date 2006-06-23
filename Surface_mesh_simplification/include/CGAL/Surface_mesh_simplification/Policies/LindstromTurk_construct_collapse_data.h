// Copyright (c) 2005, 2006 Fernando Luis Cacciola Carballal. All rights reserved.
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
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_LINDSTROMTURK_CONSTRUCT_COLLAPSE_DATA_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_LINDSTROMTURK_CONSTRUCT_COLLAPSE_DATA_H 1

#include <CGAL/Surface_mesh_simplification/TSMS_common.h>
#include <CGAL/Surface_mesh_simplification/Policies/LindstromTurk_collapse_data.h>
#include <CGAL/Surface_mesh_simplification/Policies/Detail/Lindstrom_Turk.h>


CGAL_BEGIN_NAMESPACE

namespace Triangulated_surface_mesh { namespace Simplification 
{

template<class Collapse_data_>    
class Construct_LindstromTurk_collapse_data
{
public:

  typedef Collapse_data_ Collapse_data ;
  
  typedef typename Collapse_data::TSM               TSM ;
  typedef typename Collapse_data::vertex_descriptor vertex_descriptor ;
  typedef typename Collapse_data::edge_descriptor   edge_descriptor ;
  typedef typename Collapse_data::Params            Params ;
  
  typedef shared_ptr<Collapse_data> result_type ;
  
  
public :

  result_type operator() ( vertex_descriptor const& aP 
                         , vertex_descriptor const& aQ
                         , bool                     aIsPFixed
                         , bool                     aIsQFixed
                         , edge_descriptor const&   aEdge 
                         , TSM&                     aSurface 
                         , Params const*            aParams
                         ) const
  {
    result_type r ;

    CGAL_assertion(aParams);
    
    if ( handle_assigned(aEdge) ) 
    {
      LindstromTurkImpl<Collapse_data> impl(*aParams,aP,aQ,aIsPFixed,aIsQFixed,aEdge,opposite_edge(aEdge,aSurface),aSurface);
      
      r = impl.result() ;               
    }   
    
    return r ;
  }                         
  
};    

} } // namespace Triangulated_surface_mesh::Simplification

CGAL_END_NAMESPACE

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_LINDSTROMTURK_CONSTRUCT_COLLAPSE_DATA_H //
// EOF //
 
