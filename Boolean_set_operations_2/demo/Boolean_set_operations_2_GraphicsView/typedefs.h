// Copyright (c) 2005  Tel-Aviv University (Israel).
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
// $URL: svn+ssh://fcacciola@scm.gforge.inria.fr/svn/cgal/trunk/Boolean_set_operations_2/demo/Boolean_set_operations_2/typedefs.h $
// $Id: typedefs.h 37003 2007-03-10 16:55:12Z spion $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//
#ifndef CGAL_TYPEDEFS_H
#define CGAL_TYPEDEFS_H

//
// Linear polygons
//
typedef CGAL::Simple_cartesian<double>            Linear_kernel ;
typedef CGAL::Polygon_2<Linear_kernel>            Linear_polygon;
typedef CGAL::Polygon_with_holes_2<Linear_kernel> Linear_polygon_with_holes;

typedef Linear_kernel::Point_2 Linear_point ;

//
// Circlular polygons
//

#ifdef CGAL_USE_GMP

  typedef CGAL::Gmpq                     Base_nt;

#else

  typedef CGAL::Quotient<CGAL::MP_Float> Base_nt;

#endif

typedef CGAL::Lazy_exact_nt<Base_nt> Coord_type;


struct Circular_kernel : public CGAL::Cartesian<Coord_type> {};

typedef CGAL::Gps_circle_segment_traits_2<Circular_kernel>   Circular_traits;
typedef Circular_traits::Curve_2                             Circular_curve;
typedef Circular_traits::X_monotone_curve_2                  Circular_X_monotone_curve;
typedef Circular_traits::Point_2                             Circular_point ;
typedef Circular_traits::Polygon_2                           Circular_polygon;
typedef CGAL::General_polygon_with_holes_2<Circular_polygon> Circular_polygon_with_holes;
typedef CGAL::General_polygon_set_2<Circular_traits>         Circular_polygon_set;

struct Compute_circular_X_monotone_cuve_bbox
{
  CGAL::Bbox_2 operator()( Circular_X_monotone_curve const& curve ) const 
  {
    return curve.bbox();   
  }
} ;

struct Draw_circular_X_monotone_cuve
{
  template<class Path, class Converter>
  void operator()( Circular_X_monotone_curve const& curve, Path* aPath, Converter aConvert ) const 
  {
    Linear_point lS( CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()) ) ;
    Linear_point lT( CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()) ) ;
    aPath->moveTo( aConvert( lS ) ) ;
    aPath->lineTo( aConvert( lT ) ) ;
  }
} ;
typedef CGAL::Qt::GeneralPolygonSetGraphicsItem<Circular_polygon_set,Compute_circular_X_monotone_cuve_bbox,Draw_circular_X_monotone_cuve> Circular_GI;



//
// Bezier curves typedefs
//
#ifdef CGAL_USE_CORE

typedef CGAL::CORE_algebraic_number_traits            Bezier_nt_traits;
typedef Bezier_nt_traits::Rational                    Bezier_rational;
typedef Bezier_nt_traits::Algebraic                   Bezier_algebraic;

struct Bezier_rat_kernel  : public CGAL::Cartesian<Bezier_rational>  {};
struct Bezier_alg_kernel  : public CGAL::Cartesian<Bezier_algebraic> {};

struct Bezier_traits : public CGAL::Arr_Bezier_curve_traits_2<Bezier_rat_kernel, Bezier_alg_kernel, Bezier_nt_traits> {};
  
typedef Bezier_rat_kernel::Point_2                      Bezier_rat_point;
typedef Bezier_traits::Curve_2                          Bezier_curve;
typedef Bezier_traits::X_monotone_curve_2               Bezier_X_monotone_curve;
typedef CGAL::Gps_traits_2<Bezier_traits>               Bezier_gps_traits;
typedef Bezier_gps_traits::General_polygon_2            Bezier_polygon;
typedef std::vector<Bezier_polygon>                     Bezier_polygon_vector ;
typedef Bezier_gps_traits::General_polygon_with_holes_2 Bezier_polygon_with_holes;
typedef CGAL::General_polygon_set_2<Bezier_gps_traits>  Bezier_polygon_set ;

struct BezierHelper
{
  double get_approximated_endpoint_parameter( Bezier_rat_point const& p, Bezier_curve const& curve, unsigned int xid  )
  {
    typedef typename Bezier_point::Originator_iterator Originator_iterator ;
    
    Originator_iterator org = p.get_originator (curve, xid);

    double threshold(0.001);
     
    double t_min = CGAL::to_double(org->point_bound().t_min) ;
    double t_max = CGAL::to_double(org->point_bound().t_max) ;
    
    bool can_refine = !p.is_exact();
    
    do
    {
      if ( std::abs(t_max - t_min) <= threshold )
        break ;
      
      if ( can_refine )
      {
        can_refine = p.refine();
        
        t_min = CGAL::to_double(org->point_bound().t_min) ;
        t_max = CGAL::to_double(org->point_bound().t_max) ;
      }  
    }
    while ( can_refine ) ;
    
    return ( t_min + t_max) / NT(2.0) ;
  }

  template<class OutputIterator>
  void get_control_points ( Bezier_curve const& aCurve, std::vector<Linear_point>& aQ )
  {
    int nc = aCurve.number_of_control_points() ;
    
    for ( int i = 0 ; i < nc ; ++ i )
    {
      aQ.push_back( Linear_point( CGAL::to_double( aBC.control_point(i).x() )
                                , CGAL::to_double( aBC.control_point(i).y() )
                                )
                  );
    }
  }  
  
  template<class OutputIterator>
  void clip ( Bezier_curve const& aCurve, double aS, double aT, OutputIterator aOut )
  {
    std::vector<Linear_point> lQ ;
    
    get_control_points(aCurve, std::back_inserter(lQ) ) ;
    
    int nc = aCurve.number_of_control_points() ;
    int ncc = nc - 1 ;
    
    double lAlfa = aS ;
    double lBeta = (aT-aS) / ( 1.0 - aS ) ;
    
    for ( int i = 1 ; i <= ncc ; ++ i )
    {
      for ( int j = 0 ; j < ncc ; ++ j )
        lQ[j] = lQ[j] + lAlfa * ( lQ[j+1] - lQ[j] ) ;
        
      for ( int j = nc - i ; j <= ncc ; ++ j )
        lQ[j] = lQ[j-1] + lBeta * ( lQ[j] - lQ[j-1] ) ;
    }
    
    std::copy(lQ.begin(), lQ.end(), aOut );    
  }
  
  template<class OutputIterator>
  void clip ( Bezier_X_monotone_curve const& aBXMC, OutputIterator aOut )
  {
    Bezier_curve const& lBC = aBXMC.supporting_curve();
  
    double lS = get_approximated_endpoint_parameter(aBXMC.source(), lBC, aBXMC.xid());
    double lT = get_approximated_endpoint_parameter(aBXMC.target(), lBC, aBXMC.xid());
    
    bool lFwd = lS <= lT ;
    
    double lMin = lFwd ? lS : 1.0 - lS ;
    double lMax = lFwd ? lT : 1.0 - lT ;
    
    clip(lBC, lMin, lMax, aOut ); 
  }
} ;

struct Compute_bezier_X_monotone_cuve_bbox
{
  CGAL::Bbox_2 operator()( Bezier_X_monotone_curve const& curve ) const 
  {
    std::vector<Linear_point> lQ ;
    
    get_control_points(aCurve, std::back_inserter(lQ) ) ;
    
    return CGAL::bbox_2(lQ.begin(),lQ.end());
  }
} ;

struct Draw_bezier_X_monotone_cuve
{
  template<class Path, class Converter>
  void operator()( Bezier_X_monotone_curve const& aBXMC, Path* aPath, Converter aConvert ) const 
  {
    std::vector<Linear_point> lQ ;
    clip(aBXMC,std::back_inserter(lQ));
    if ( lQ.size() == 2 )
    {
    }
    else if ( lQ.size() == 4 )
    {
    }
    
    
//    aPainter->moveTo( aConvert( curve.source() ) ) ;
//    aPainter->lineTo( aConvert( curve.source() ) ) ;
  }
} ;

typedef CGAL::Qt::GeneralPolygonSetGraphicsItem<Bezier_polygon_set,Compute_bezier_X_monotone_cuve_bbox,Draw_bezier_X_monotone_cuve> Bezier_GI;


#endif

#endif
