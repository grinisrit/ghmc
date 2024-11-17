// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>

#include <noa/3rdparty/tnl-noa/src/TNL/Cuda/CudaCallable.h>

namespace noa::TNL {
namespace Matrices {

/**
 * \brief Accessor for multidiagonal matrix elements.
 *
 * \tparam Real is a type of matrix elements values.
 * \tparam Index is a type of matrix elements column indexes.
 */
template< typename Real, typename Index >
class MultidiagonalMatrixElement
{
public:
   /**
    * \brief Type of matrix elements values.
    */
   using RealType = Real;

   /**
    * \brief Type of matrix elements column indexes.
    */
   using IndexType = Index;

   /**
    * \brief Constructor.
    *
    * \param value is matrix element value.
    * \param rowIdx is row index of the matrix element.
    * \param columnIdx is a column index of the matrix element.
    * \param localIdx is the rank of the non-zero elements in the matrix row.
    */
   __cuda_callable__
   MultidiagonalMatrixElement( RealType& value, IndexType rowIdx, IndexType columnIdx, IndexType localIdx )
   : value_( value ), rowIdx( rowIdx ), columnIdx( columnIdx ), localIdx( localIdx )
   {}

   /**
    * \brief Returns reference on matrix element value.
    *
    * \return reference on matrix element value.
    */
   __cuda_callable__
   RealType&
   value()
   {
      return value_;
   }

   /**
    * \brief Returns constant reference on matrix element value.
    *
    * \return constant reference on matrix element value.
    */
   __cuda_callable__
   const RealType&
   value() const
   {
      return value_;
   }

   /**
    * \brief Returns constant reference on matrix element column index.
    *
    * \return constant reference on matrix element column index.
    */
   __cuda_callable__
   const IndexType&
   rowIndex() const
   {
      return rowIdx;
   }

   /**
    * \brief Returns constant reference on matrix element column index.
    *
    * \return constant reference on matrix element column index.
    */
   __cuda_callable__
   const IndexType&
   columnIndex() const
   {
      return columnIdx;
   }

   /**
    * \brief Returns constant reference on the rank of the non-zero matrix element in the row.
    *
    * \return constant reference on the rank of the non-zero matrix element in the row.
    */
   __cuda_callable__
   const IndexType&
   localIndex() const
   {
      return localIdx;
   }

protected:
   RealType& value_;

   // NOTE: this cannot be a reference to avoid binding to temporary objects
   IndexType rowIdx;

   IndexType columnIdx;

   // NOTE: this cannot be a reference to avoid binding to temporary objects
   IndexType localIdx;
};

}  // namespace Matrices
}  // namespace noa::TNL
