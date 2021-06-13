/**
 *    MSA 11.04.11
 *    Implementing precompiled header for Boost functionality.
 *    Boost::geometry in particular produces a LOT of compilation warnings, which are harmless - 
 *    unreferenced formal parameters and the like - but which clog the output window with noise.
 *    By precompiling them once we can reduce compilation noise and compilation time.
 */

#ifndef BoostPrecomp_H
#define BoostPrecomp_H

#include "boost/geometry/geometry.hpp"
#include "boost/geometry/core/tags.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/geometries/point.hpp"
#include "boost/geometry/geometries/cartesian2d.hpp"
#include "boost/geometry/geometries/adapted/c_array_cartesian.hpp"
#include "boost/geometry/geometries/adapted/std_as_linestring.hpp"
#include "boost/geometry/multi/multi.hpp"
#include "boost/geometry/extensions/gis/io/wkt/read_wkt.hpp"

#include "boost/geometry/algorithms/overlay/enrich_intersection_points.hpp"
#include "boost/numeric/ublas/matrix.hpp"

#endif
