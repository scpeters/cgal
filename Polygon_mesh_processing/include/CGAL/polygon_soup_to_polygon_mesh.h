// Copyright (c) 2009-2013 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
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
// Author(s)     : Laurent Rineau and Ilker O. Yaz

#ifndef CGAL_POLYGON_SOUP_TO_POLYHEDRON
#define CGAL_POLYGON_SOUP_TO_POLYHEDRON

#include <CGAL/Modifier_base.h>
#include <CGAL/IO/generic_print_polyhedron.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/boost/graph/Euler_operations.h>

namespace CGAL
{
namespace Polygon_mesh_processing
{
namespace internal
{
template <typename PM
        , typename Point
        , typename Polygon>
class Polygon_soup_to_polygon_mesh
  : public CGAL::Modifier_base< PM >
{
  const std::vector<Point>& _points;
  const std::vector<Polygon>& _polygons;

  typedef typename PM::Point Point_3;

  typedef typename boost::graph_traits<PM>::vertex_descriptor vertex_descriptor;
  typedef typename boost::graph_traits<PM>::halfedge_descriptor halfedge_descriptor;
  typedef typename boost::graph_traits<PM>::face_descriptor face_descriptor;

public:
  /**
  * The constructor for modifier object.
  * @param points points of the soup of polygons.
  * @param polygons each element in the vector describes a polygon using the index of the points in the vector.
  */
  Polygon_soup_to_polygon_mesh(const std::vector<Point>& points,
                               const std::vector<Polygon>& polygons)
    : _points(points),
      _polygons(polygons)
  { }

  void operator()(PM& pmesh)
  {
    std::vector<vertex_descriptor> vertices(_points.size());
    for (std::size_t i = 0, end = _points.size(); i < end; ++i)
    {
      Point pi(_points[i].x(), _points[i].y(), _points[i].z());
      vertices[i] = add_vertex(pi, pmesh);
    }

    for (std::size_t i = 0, end = _polygons.size(); i < end; ++i)
    {
      const Polygon& polygon = _polygons[i];
      const std::size_t size = polygon.size();

      std::vector<vertex_descriptor> vr(size); //vertex range
      vr.resize(size);
      for (std::size_t j = 0; j < size; ++j)
        vr[j] = vertices[polygon[j] ];

      CGAL_assertion_code(face_descriptor fd = )
      CGAL::Euler::add_face(vr, pmesh);
      CGAL_assertion(fd != boost::graph_traits<PM>::null_face());
    }
  }
};
}//end namespace internal

  /**
  * \ingroup PkgPolygonMeshProcessing
  * builds a polygon mesh from a soup of polygons.
  * @pre the input polygon soup is consistently oriented.
  * @tparam PolygonMesh a model of `MutableFaceGraph`
  * @tparam Point a point type that has the three operators `x()`, `y()`, and `z()`.
  * @tparam Polygon a `std::vector<std::size_t>` corresponding to the indices
  *         of points in `points`
  */
  template<class PolygonMesh, class Point, class Polygon>
  void polygon_soup_to_polygon_mesh(
    const std::vector<Point>& points,
    const std::vector<Polygon>& polygons,
    PolygonMesh& out)
  {
    internal::Polygon_soup_to_polygon_mesh<PolygonMesh, Point, Polygon>
      converter(points, polygons);
    converter(out);
  }

}//end namespace Polygon_mesh_processing

}// end namespace CGAL

#endif
