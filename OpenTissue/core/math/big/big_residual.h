#ifndef OPENTISSUE_CORE_MATH_BIG_RESIDUAL_H
#define OPENTISSUE_CORE_MATH_BIG_RESIDUAL_H
//
// OpenTissue Template Library
// - A generic toolbox for physics-based modeling and simulation.
// Copyright (C) 2008 Department of Computer Science, University of Copenhagen.
//
// OTTL is licensed under zlib: http://opensource.org/licenses/zlib-license.php
//
#include <OpenTissue/configuration.h>

#include <OpenTissue/core/math/big/big_types.h>

#include <cassert>

namespace OpenTissue
{
  namespace math
  {
    namespace big
    {
      /**
      * Compute r  = b - prod(A,x).
      *
      * @param A      The matrix.
      * @param x      The solution vector.
      * @param b      The right hand side vector.
      * @param r      Upon return this argument holds the residual value of the matrix equation A x = b.
      */
      template<typename T>
      inline void residual(
          boost::numeric::ublas::compressed_matrix<T> const & A
        , boost::numeric::ublas::vector<T> const & x
        , boost::numeric::ublas::vector<T> const & b
        , boost::numeric::ublas::vector<T>       & r
        )
      {
        typedef boost::numeric::ublas::vector<T> vector_type;
        typedef typename vector_type::size_type  size_type;
        typedef typename vector_type::value_type real_type;

        assert(A.size1()>0            || !"residual(): A was empty"            );
        assert(A.size2()>0            || !"residual(): A was empty"            );
        assert(A.size2() ==  x.size() || !"residual(): incompatible dimensions");
        assert(A.size1() ==  b.size() || !"residual(): incompatible dimensions");

        if(r.size() != b.size())
          r.resize(b.size(), false );

        //
        //  Example of compressed matrix format:
        //      
        //    0 x y 0
        //    0 0 z 0
        //    w 0 0 0
        //
        //   value_data: [ x y z w ]
        //
        //    Note this is simply all non-zero elements of the sparse matrix stored in a row-by-row wise manner.
        //
        //   index1_data: [ 0 2 3 ]     
        //
        //    Note that the number of elements is equal to the number of rows in the
        //    sparse matrix. Each element points to the index in value_data that
        //    corresponds to the starting element of the corresponding row.
        //
        //   index2_data: [ 1 2 2 0 ]     
        //
        //    Note this array have the same dimension as value_data. Each element in index2_data
        //    stores the corresponind column index of the matching element in value_data.
        //
        size_type row_end = A.filled1 () - 1;
        for (size_type i = 0; i < row_end; ++ i) 
        {
          size_type begin = A.index1_data()[i];
          size_type end   = A.index1_data()[i + 1];
          real_type t = b(i);
          for (size_type j = begin; j < end; ++ j)
            t -= A.value_data()[j] * x(  A.index2_data()[j]   );
          r (i) = t;
        }
      }

    } // end of namespace big
  } // end of namespace math
} // end of namespace OpenTissue

// OPENTISSUE_CORE_MATH_BIG_RESIDUAL_H
#endif
