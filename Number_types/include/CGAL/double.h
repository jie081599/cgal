// Copyright (c) 1999  Utrecht University (The Netherlands),
// ETH Zurich (Switzerland), Freie Universitaet Berlin (Germany),
// INRIA Sophia-Antipolis (France), Martin-Luther-University Halle-Wittenberg
// (Germany), Max-Planck-Institute Saarbruecken (Germany), RISC Linz (Austria),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
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
// Author(s)     : Geert-Jan Giezeman, Michael Hemmer

#ifndef CGAL_DOUBLE_H
#define CGAL_DOUBLE_H

#include <CGAL/number_type_basic.h>

#include <utility>
#include <cmath>
#include <math.h> // for nextafter
#include <limits>


#ifdef _MSC_VER
#include <float.h>
#endif

#ifdef CGAL_CFG_IEEE_754_BUG
#  include <CGAL/IEEE_754_unions.h>
#endif
#ifdef __sgi
#  include <fp_class.h>
#endif



CGAL_BEGIN_NAMESPACE
template<> struct Number_type_traits<double> {
  typedef Tag_false  Has_gcd;
  typedef Tag_true   Has_division;
  typedef Tag_true   Has_sqrt;

  typedef Tag_false  Has_exact_ring_operations;
  typedef Tag_false  Has_exact_division;
  typedef Tag_false  Has_exact_sqrt;
};

#ifdef __sgi

template<>
class Is_valid< double > 
  : public Unary_function< double, bool > {
  public :
    bool operator()( const double& x ) const {
      switch (fp_class_d(x)) {
      case FP_POS_NORM:
      case FP_NEG_NORM:
      case FP_POS_ZERO:
      case FP_NEG_ZERO:
      case FP_POS_INF:
      case FP_NEG_INF:
      case FP_POS_DENORM:
      case FP_NEG_DENORM:
          return true;
      case FP_SNAN:
      case FP_QNAN:
          return false;
      }
      return false; // NOT REACHED
    }  
};

#elif defined CGAL_CFG_IEEE_754_BUG

#define CGAL_EXPONENT_DOUBLE_MASK   0x7ff00000
#define CGAL_MANTISSA_DOUBLE_MASK   0x000fffff

inline
bool
is_finite_by_mask_double(unsigned int h)
{
  unsigned int e = h & CGAL_EXPONENT_DOUBLE_MASK;
  return ( ( e ^ CGAL_EXPONENT_DOUBLE_MASK ) != 0 );
}

inline
bool
is_nan_by_mask_double(unsigned int h, unsigned int l)
{
  if ( is_finite_by_mask_double(h) )
      return false;
  return (( h & CGAL_MANTISSA_DOUBLE_MASK ) != 0) || (( l & 0xffffffff ) != 0);
}

template<>
class Is_valid< double > 
  : public Unary_function< double, bool > {
  public :
    bool operator()( const double& x ) const{
      double d = x;
      IEEE_754_double* p = reinterpret_cast<IEEE_754_double*>(&d);
      return ! ( is_nan_by_mask_double( p->c.H, p->c.L ));
    }  
};

#else

#ifdef _MSC_VER

template<>
class Is_valid< double > 
  : public Unary_function< double, bool > {
  public :
    bool operator()( const double& x ) const {
      return ! _isnan(x);
    }  
};

#else

template<>
class Is_valid< double > 
  : public Unary_function< double, bool > {
  public :
    bool operator()( const double& x ) const {
      return (x == x);
    }  
};

#endif 

#endif

template <> class Algebraic_structure_traits< double >
  : public Algebraic_structure_traits_base< double, 
                                            Field_with_kth_root_tag >  {
  public:
    typedef Tag_false            Is_exact;
            
    class Sqrt 
      : public Unary_function< Algebraic_structure, Algebraic_structure > {
      public:
        Algebraic_structure operator()( const Algebraic_structure& x ) const {
          return CGAL_CLIB_STD::sqrt( x );
        }
    };
    
    class Kth_root 
      : public Binary_function<int, Algebraic_structure, Algebraic_structure> {
      public:
        Algebraic_structure operator()( int k, 
                                        const Algebraic_structure& x) const {
          CGAL_precondition_msg( k > 0, "'k' must be positive for k-th roots");
          return CGAL_CLIB_STD::pow(x, 1.0 / double(k));
        }
    };
    
};

template <> class Real_embeddable_traits< double > 
  : public Real_embeddable_traits_base< double > {
  public:

// GCC is faster with std::fabs().
#ifdef __GNUG__
    class Abs 
      : public Unary_function< Real_embeddable, Real_embeddable > {
      public:        
        Real_embeddable operator()( const Real_embeddable& x ) const {
          return CGAL_CLIB_STD::fabs( x );
        }
    };    
#endif

    typedef INTERN_RET::To_double_by_conversion< Real_embeddable >
                                                                  To_double;      
    typedef INTERN_RET::To_interval_by_conversion< Real_embeddable >
                                                                  To_interval;    
                                                                  
// Is_finite depends on platform
#ifdef __sgi
    class Is_finite
      : public Unary_function< Real_embeddable, bool > {
      public :
        bool operator()( const Real_embeddable& x ) const {
          switch (fp_class_d(x)) {
          case FP_POS_NORM:
          case FP_NEG_NORM:
          case FP_POS_ZERO:
          case FP_NEG_ZERO:
          case FP_POS_DENORM:
          case FP_NEG_DENORM:
              return true;
          case FP_SNAN:
          case FP_QNAN:
          case FP_POS_INF:
          case FP_NEG_INF:
              return false;
          }
          return false; // NOT REACHED          
        }
    };                                                                      

#elif defined CGAL_CFG_IEEE_754_BUG
    class Is_finite
      : public Unary_function< Real_embeddable, bool > {
      public :
        bool operator()( const Real_embeddable& x ) const {
          Real_embeddable d = x;
          IEEE_754_double* p = reinterpret_cast<IEEE_754_double*>(&d);
          return is_finite_by_mask_double( p->c.H );
        }
    };                                                                      
#elif defined CGAL_CFG_NUMERIC_LIMITS_BUG
    class Is_finite
      : public Unary_function< Real_embeddable, bool > {
      public :
        bool operator()( const Real_embeddable& x ) const {
           return (x == x) && (is_valid(x-x));
        }
    };                                                                      
#else
    class Is_finite
      : public Unary_function< Real_embeddable, bool > {
      public :
        bool operator()( const Real_embeddable& x ) const {
          return (x != std::numeric_limits<Real_embeddable>::infinity()) 
              && (-x != std::numeric_limits<Real_embeddable>::infinity())
              && is_valid(x);
      }
    };                                                                      
#endif
};

inline
io_Read_write
io_tag(double)
{ return io_Read_write(); }

inline
double
nextafter(double d1, double d2)
{
#ifdef CGAL_CFG_NO_NEXTAFTER
  return _nextafter(d1, d2); // works at least for VC++-7.1
#else
  return ::nextafter(d1,d2);
#endif
}

CGAL_END_NAMESPACE

#endif // CGAL_DOUBLE_H
