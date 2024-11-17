// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Allocators/Default.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Devices/Host.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/DenseMatrixRowView.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/MatrixView.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/MatrixType.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Segments/Ellpack.h>

namespace noa::TNL {
namespace Matrices {

/**
 * \brief Implementation of dense matrix view.
 *
 * It serves as an accessor to \ref DenseMatrix for example when passing the
 * matrix to lambda functions. DenseMatrix view can be also created in CUDA kernels.
 *
 * \tparam Real is a type of matrix elements.
 * \tparam Device is a device where the matrix is allocated.
 * \tparam Index is a type for indexing of the matrix elements.
 * \tparam MatrixElementsOrganization tells the ordering of matrix elements in memory. It is either
 *         \ref TNL::Algorithms::Segments::RowMajorOrder
 *         or \ref TNL::Algorithms::Segments::ColumnMajorOrder.
 *
 * See \ref DenseMatrix.
 */
template< typename Real = double,
          typename Device = Devices::Host,
          typename Index = int,
          ElementsOrganization Organization = Algorithms::Segments::DefaultElementsOrganization< Device >::getOrganization() >
class DenseMatrixView : public MatrixView< Real, Device, Index >
{
protected:
   using BaseType = MatrixView< Real, Device, Index >;
   using SegmentsType = Algorithms::Segments::
      Ellpack< Device, Index, typename Allocators::Default< Device >::template Allocator< Index >, Organization, 1 >;
   using SegmentsViewType = typename SegmentsType::ViewType;
   using SegmentViewType = typename SegmentsType::SegmentViewType;

public:
   /**
    * \brief The type of matrix elements.
    */
   using RealType = Real;

   /**
    * \brief The device where the matrix is allocated.
    */
   using DeviceType = Device;

   /**
    * \brief The type used for matrix elements indexing.
    */
   using IndexType = Index;

   /**
    * \brief Matrix elements organization getter.
    *
    * \return matrix elements organization - RowMajorOrder of ColumnMajorOrder.
    */
   static constexpr ElementsOrganization
   getOrganization()
   {
      return Organization;
   }

   /**
    * \brief Matrix elements container view type.
    *
    * Use this for embedding of the matrix elements values.
    */
   using ValuesViewType = typename BaseType::ValuesView;

   /**
    * \brief Matrix elements container view type.
    *
    * Use this for embedding of the matrix elements values.
    */
   using ConstValuesViewType = typename ValuesViewType::ConstViewType;

   /**
    * \brief Matrix view type.
    *
    * See \ref DenseMatrixView.
    */
   using ViewType = DenseMatrixView< Real, Device, Index, Organization >;

   /**
    * \brief Matrix view type for constant instances.
    *
    * See \ref DenseMatrixView.
    */
   using ConstViewType = DenseMatrixView< std::add_const_t< Real >, Device, Index, Organization >;

   /**
    * \brief Type for accessing matrix row.
    */
   using RowView = DenseMatrixRowView< SegmentViewType, ValuesViewType >;

   /**
    * \brief Type for accessing immutable matrix row.
    */
   using ConstRowView = typename RowView::ConstRowView;

   /**
    * \brief Helper type for getting self type or its modifications.
    */
   template< typename _Real = Real, typename _Device = Device, typename _Index = Index >
   using Self = DenseMatrixView< _Real, _Device, _Index >;

   /**
    * \brief Constructor without parameters.
    */
   __cuda_callable__
   DenseMatrixView();

   /**
    * \brief Constructor with matrix dimensions and values.
    *
    * Organization of matrix elements values in
    *
    * \param rows number of matrix rows.
    * \param columns number of matrix columns.
    * \param values is vector view with matrix elements values.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_constructor.cpp
    * \par Output
    * \include DenseMatrixViewExample_constructor.out
    */
   __cuda_callable__
   DenseMatrixView( IndexType rows, IndexType columns, const ValuesViewType& values );

   /**
    * \brief Constructor with matrix dimensions and values.
    *
    * Organization of matrix elements values in
    *
    * \param rows number of matrix rows.
    * \param columns number of matrix columns.
    * \param values is vector view with matrix elements values.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_constructor.cpp
    * \par Output
    * \include DenseMatrixViewExample_constructor.out
    */
   template< typename Real_ >
   __cuda_callable__
   DenseMatrixView( IndexType rows, IndexType columns, const Containers::VectorView< Real_, Device, Index >& values );

   /**
    * \brief Copy constructor.
    *
    * \param matrix is the source matrix view.
    */
   __cuda_callable__
   DenseMatrixView( const DenseMatrixView& matrix ) = default;

   /**
    * \brief Returns a modifiable dense matrix view.
    *
    * \return dense matrix view.
    */
   __cuda_callable__
   ViewType
   getView();

   /**
    * \brief Returns a non-modifiable dense matrix view.
    *
    * \return dense matrix view.
    */
   __cuda_callable__
   ConstViewType
   getConstView() const;

   /**
    * \brief Returns string with serialization type.
    *
    * The string has a form \e `Matrices::DenseMatrix< RealType,  [any_device], IndexType, [any_allocator], true/false >`.
    *
    * \return \e String with the serialization type.
    */
   static std::string
   getSerializationType();

   /**
    * \brief Returns string with serialization type.
    *
    * See \ref DenseMatrixView::getSerializationType.
    *
    * \return \e String with the serialization type.
    */
   std::string
   getSerializationTypeVirtual() const override;

   /**
    * \brief Compute capacities of all rows.
    *
    * The row capacities are not stored explicitly and must be computed.
    *
    * \param rowCapacities is a vector where the row capacities will be stored.
    */
   template< typename Vector >
   void
   getRowCapacities( Vector& rowCapacities ) const;

   /**
    * \brief Computes number of non-zeros in each row.
    *
    * \param rowLengths is a vector into which the number of non-zeros in each row
    * will be stored.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_getCompressedRowLengths.cpp
    * \par Output
    * \include DenseMatrixViewExample_getCompressedRowLengths.out
    */
   template< typename Vector >
   void
   getCompressedRowLengths( Vector& rowLengths ) const;

   /**
    * \brief Returns number of all matrix elements.
    *
    * This method is here mainly for compatibility with sparse matrices since
    * the number of all matrix elements is just number of rows times number of
    * columns.
    *
    * \return number of all matrix elements.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_getElementsCount.cpp
    * \par Output
    * \include DenseMatrixViewExample_getElementsCount.out
    */
   IndexType
   getAllocatedElementsCount() const;

   /**
    * \brief Returns number of non-zero matrix elements.
    *
    * \return number of all non-zero matrix elements.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_getElementsCount.cpp
    * \par Output
    * \include DenseMatrixViewExample_getElementsCount.out
    */
   IndexType
   getNonzeroElementsCount() const override;

   /**
    * \brief Constant getter of simple structure for accessing given matrix row.
    *
    * \param rowIdx is matrix row index.
    *
    * \return RowView for accessing given matrix row.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_getConstRow.cpp
    * \par Output
    * \include DenseMatrixViewExample_getConstRow.out
    *
    * See \ref DenseMatrixRowView.
    */
   __cuda_callable__
   ConstRowView
   getRow( IndexType rowIdx ) const;

   /**
    * \brief Non-constant getter of simple structure for accessing given matrix row.
    *
    * \param rowIdx is matrix row index.
    *
    * \return RowView for accessing given matrix row.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_getRow.cpp
    * \par Output
    * \include DenseMatrixExample_getRow.out
    *
    * See \ref DenseMatrixRowView.
    */
   __cuda_callable__
   RowView
   getRow( IndexType rowIdx );

   /**
    * \brief Sets all matrix elements to value \e v.
    *
    * \param v is value all matrix elements will be set to.
    */
   void
   setValue( const RealType& v );

   /**
    * \brief Returns non-constant reference to element at row \e row and column column.
    *
    * Since this method returns reference to the element, it cannot be called across
    * different address spaces. It means that it can be called only form CPU if the matrix
    * is allocated on CPU or only from GPU kernels if the matrix is allocated on GPU.
    *
    * \param row is a row index of the element.
    * \param column is a columns index of the element.
    * \return reference to given matrix element.
    */
   __cuda_callable__
   Real&
   operator()( IndexType row, IndexType column );

   /**
    * \brief Returns constant reference to element at row \e row and column column.
    *
    * Since this method returns reference to the element, it cannot be called across
    * different address spaces. It means that it can be called only form CPU if the matrix
    * is allocated on CPU or only from GPU kernels if the matrix is allocated on GPU.
    *
    * \param row is a row index of the element.
    * \param column is a columns index of the element.
    * \return reference to given matrix element.
    */
   __cuda_callable__
   const Real&
   operator()( IndexType row, IndexType column ) const;

   /**
    * \brief Sets element at given \e row and \e column to given \e value.
    *
    * This method can be called from the host system (CPU) no matter
    * where the matrix is allocated. If the matrix is allocated on GPU this method
    * can be called even from device kernels. If the matrix is allocated in GPU device
    * this method is called from CPU, it transfers values of each matrix element separately and so the
    * performance is very low. For higher performance see. \ref DenseMatrix::getRow
    * or \ref DenseMatrix::forElements and \ref DenseMatrix::forAllElements.
    *
    * \param row is row index of the element.
    * \param column is columns index of the element.
    * \param value is the value the element will be set to.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_setElement.cpp
    * \par Output
    * \include DenseMatrixExample_setElement.out
    */
   __cuda_callable__
   void
   setElement( IndexType row, IndexType column, const RealType& value );

   /**
    * \brief Add element at given \e row and \e column to given \e value.
    *
    * This method can be called from the host system (CPU) no matter
    * where the matrix is allocated. If the matrix is allocated on GPU this method
    * can be called even from device kernels. If the matrix is allocated in GPU device
    * this method is called from CPU, it transfers values of each matrix element separately and so the
    * performance is very low. For higher performance see. \ref DenseMatrix::getRow
    * or \ref DenseMatrix::forElements and \ref DenseMatrix::forAllElements.
    *
    * \param row is row index of the element.
    * \param column is columns index of the element.
    * \param value is the value the element will be set to.
    * \param thisElementMultiplicator is multiplicator the original matrix element
    *   value is multiplied by before addition of given \e value.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_addElement.cpp
    * \par Output
    * \include DenseMatrixExample_addElement.out
    *
    */
   __cuda_callable__
   void
   addElement( IndexType row, IndexType column, const RealType& value, const RealType& thisElementMultiplicator = 1.0 );

   /**
    * \brief Returns value of matrix element at position given by its row and column index.
    *
    * This method can be called from the host system (CPU) no matter
    * where the matrix is allocated. If the matrix is allocated on GPU this method
    * can be called even from device kernels. If the matrix is allocated in GPU device
    * this method is called from CPU, it transfers values of each matrix element separately and so the
    * performance is very low. For higher performance see. \ref DenseMatrix::getRow
    * or \ref DenseMatrix::forElements and \ref DenseMatrix::forAllElements.
    *
    * \param row is a row index of the matrix element.
    * \param column i a column index of the matrix element.
    *
    * \return value of given matrix element.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_getElement.cpp
    * \par Output
    * \include DenseMatrixExample_getElement.out
    *
    */
   __cuda_callable__
   Real
   getElement( IndexType row, IndexType column ) const;

   /**
    * \brief Method for performing general reduction on matrix rows.
    *
    * \tparam Fetch is a type of lambda function for data fetch declared as
    *
    * ```
    * auto fetch = [=] __cuda_callable__ ( IndexType rowIdx, IndexType columnIdx, RealType elementValue ) -> FetchValue { ... };
    * ```
    *
    *   The return type of this lambda can be any non void.
    * \tparam Reduce is a type of lambda function for reduction declared as
    *
    * ```
    * auto reduce = [=] __cuda_callable__ ( const FetchValue& v1, const FetchValue& v2 ) -> FetchValue { ... };
    * ```
    *
    * \tparam Keep is a type of lambda function for storing results of reduction in each row.
    *          It is declared as
    *
    * ```
    * auto keep = [=] __cuda_callable__ ( IndexType rowIdx, const RealType& value ) { ... };
    * ```
    *
    * \tparam FetchValue is type returned by the Fetch lambda function.
    *
    * \param begin defines beginning of the range [begin,end) of rows to be processed.
    * \param end defines ending of the range [begin,end) of rows to be processed.
    * \param fetch is an instance of lambda function for data fetch.
    * \param reduce is an instance of lambda function for reduction.
    * \param keep in an instance of lambda function for storing results.
    * \param identity is the [identity element](https://en.wikipedia.org/wiki/Identity_element)
    *                 for the reduction operation, i.e. element which does not
    *                 change the result of the reduction.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_reduceRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_reduceRows.out
    */
   template< typename Fetch, typename Reduce, typename Keep, typename FetchReal >
   void
   reduceRows( IndexType begin, IndexType end, Fetch& fetch, const Reduce& reduce, Keep& keep, const FetchReal& identity );

   /**
    * \brief Method for performing general reduction on matrix rows for constant instances.
    *
    * \tparam Fetch is a type of lambda function for data fetch declared as
    *
    * ```
    * auto fetch = [=] __cuda_callable__ ( IndexType rowIdx, IndexType columnIdx, RealType elementValue ) -> FetchValue { ... };
    * ```
    *
    *  The return type of this lambda can be any non void.
    * \tparam Reduce is a type of lambda function for reduction declared as
    *
    * ```
    * auto reduce = [=] __cuda_callable__ ( const FetchValue& v1, const FetchValue& v2 ) -> FetchValue { ... };
    * ```
    *
    * \tparam Keep is a type of lambda function for storing results of reduction in each row.
    *          It is declared as
    *
    * ```
    * auto keep = [=] __cuda_callable__ ( IndexType rowIdx, const RealType& value ) { ... };
    * ```
    *
    * \tparam FetchValue is type returned by the Fetch lambda function.
    *
    * \param begin defines beginning of the range [begin,end) of rows to be processed.
    * \param end defines ending of the range [begin,end) of rows to be processed.
    * \param fetch is an instance of lambda function for data fetch.
    * \param reduce is an instance of lambda function for reduction.
    * \param keep in an instance of lambda function for storing results.
    * \param identity is the [identity element](https://en.wikipedia.org/wiki/Identity_element)
    *                 for the reduction operation, i.e. element which does not
    *                 change the result of the reduction.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_reduceRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_reduceRows.out
    */
   template< typename Fetch, typename Reduce, typename Keep, typename FetchReal >
   void
   reduceRows( IndexType begin, IndexType end, Fetch& fetch, const Reduce& reduce, Keep& keep, const FetchReal& identity )
      const;

   /**
    * \brief Method for performing general reduction on ALL matrix rows.
    *
    * \tparam Fetch is a type of lambda function for data fetch declared as
    *
    * ```
    * auto fetch = [=] __cuda_callable__ ( IndexType rowIdx, IndexType columnIdx, RealType elementValue ) -> FetchValue
    * ```
    *
    * The return type of this lambda can be any non void.
    * \tparam Reduce is a type of lambda function for reduction declared as
    *
    * ```
    * auto reduce = [=] __cuda_callable__ ( const FetchValue& v1, const FetchValue& v2 ) -> FetchValue { ... };
    * ```
    *
    * \tparam Keep is a type of lambda function for storing results of reduction in each row.
    *   It is declared as
    *
    * ```
    * auto keep = [=] __cuda_callable__ ( IndexType rowIdx, const RealType& value ) { ... };
    * ```
    *
    * \tparam FetchValue is type returned by the Fetch lambda function.
    *
    * \param fetch is an instance of lambda function for data fetch.
    * \param reduce is an instance of lambda function for reduction.
    * \param keep in an instance of lambda function for storing results.
    * \param identity is the [identity element](https://en.wikipedia.org/wiki/Identity_element)
    *                 for the reduction operation, i.e. element which does not
    *                 change the result of the reduction.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_reduceAllRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_reduceAllRows.out
    */
   template< typename Fetch, typename Reduce, typename Keep, typename FetchReal >
   void
   reduceAllRows( Fetch& fetch, const Reduce& reduce, Keep& keep, const FetchReal& identity );

   /**
    * \brief Method for performing general reduction on ALL matrix rows for constant instances.
    *
    * \tparam Fetch is a type of lambda function for data fetch declared as
    *
    * ```
    * auto fetch = [=] __cuda_callable__ ( IndexType rowIdx, IndexType columnIdx, RealType elementValue ) -> FetchValue { ... };
    * ```
    *
    *  The return type of this lambda can be any non void.
    * \tparam Reduce is a type of lambda function for reduction declared as
    *
    * ```
    * auto reduce = [=] __cuda_callable__ ( const FetchValue& v1, const FetchValue& v2 ) -> FetchValue { ... };
    * ```
    *
    * \tparam Keep is a type of lambda function for storing results of reduction in each row.
    *  It is declared as
    *
    * ```
    * auto keep = [=] __cuda_callable__ ( IndexType rowIdx, const RealType& value ) { ... };
    * ```
    *
    * \tparam FetchValue is type returned by the Fetch lambda function.
    *
    * \param fetch is an instance of lambda function for data fetch.
    * \param reduce is an instance of lambda function for reduction.
    * \param keep in an instance of lambda function for storing results.
    * \param identity is the [identity element](https://en.wikipedia.org/wiki/Identity_element)
    *                 for the reduction operation, i.e. element which does not
    *                 change the result of the reduction.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_reduceAllRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_reduceAllRows.out
    */
   template< typename Fetch, typename Reduce, typename Keep, typename FetchReal >
   void
   reduceAllRows( Fetch& fetch, const Reduce& reduce, Keep& keep, const FetchReal& identity ) const;

   /**
    * \brief Method for iteration over all matrix rows for constant instances.
    *
    * \tparam Function is type of lambda function that will operate on matrix elements.
    *    It is should have form like
    *
    * ```
    * auto function = [=] __cuda_callable__ ( IndexType rowIdx, IndexType columnIdx, IndexType columnIdx, const RealType& value
    * ) { ... };
    * ```
    *
    *  The column index repeats twice only for compatibility with sparse matrices.
    *
    * \param begin defines beginning of the range [begin,end) of rows to be processed.
    * \param end defines ending of the range [begin,end) of rows to be processed.
    * \param function is an instance of the lambda function to be called in each row.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_forRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_forRows.out
    */
   template< typename Function >
   void
   forElements( IndexType begin, IndexType end, Function&& function ) const;

   /**
    * \brief Method for iteration over all matrix rows for non-constant instances.
    *
    * \tparam Function is type of lambda function that will operate on matrix elements.
    *    It is should have form like
    *
    * ```
    * auto function = [=] __cuda_callable__ ( IndexType rowIdx, IndexType columnIdx, IndexType columnIdx, RealType& value ) {
    * ... };
    * ```
    *
    *  The column index repeats twice only for compatibility with sparse matrices.
    *
    * \param begin defines beginning of the range [begin,end) of rows to be processed.
    * \param end defines ending of the range [begin,end) of rows to be processed.
    * \param function is an instance of the lambda function to be called in each row.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_forRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_forRows.out
    */
   template< typename Function >
   void
   forElements( IndexType begin, IndexType end, Function&& function );

   /**
    * \brief This method calls \e forElements for all matrix rows.
    *
    * See \ref DenseMatrix::forElements.
    *
    * \tparam Function is a type of lambda function that will operate on matrix elements.
    * \param function  is an instance of the lambda function to be called in each row.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_forAllElements.cpp
    * \par Output
    * \include DenseMatrixViewExample_forAllElements.out
    */
   template< typename Function >
   void
   forAllElements( Function&& function ) const;

   /**
    * \brief This method calls \e forElements for all matrix rows.
    *
    * See \ref DenseMatrix::forAllElements.
    *
    * \tparam Function is a type of lambda function that will operate on matrix elements.
    * \param function  is an instance of the lambda function to be called in each row.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixExample_forAllElements.cpp
    * \par Output
    * \include DenseMatrixExample_forAllElements.out
    */
   template< typename Function >
   void
   forAllElements( Function&& function );

   /**
    * \brief Method for parallel iteration over matrix rows from interval [ \e begin, \e end).
    *
    * In each row, given lambda function is performed. Each row is processed by at most one thread unlike the method
    * \ref DenseMatrix::forElements where more than one thread can be mapped to each row.
    *
    * \tparam Function is type of the lambda function.
    *
    * \param begin defines beginning of the range [ \e begin,\e end ) of rows to be processed.
    * \param end defines ending of the range [ \e begin, \e end ) of rows to be processed.
    * \param function is an instance of the lambda function to be called for each row.
    *
    * ```
    * auto function = [] __cuda_callable__ ( RowView& row ) mutable { ... };
    * ```
    *
    * \e RowView represents matrix row - see \ref TNL::Matrices::DenseMatrix::RowView.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixExample_forRows.cpp
    * \par Output
    * \include DenseMatrixExample_forRows.out
    */
   template< typename Function >
   void
   forRows( IndexType begin, IndexType end, Function&& function );

   /**
    * \brief Method for parallel iteration over matrix rows from interval [ \e begin, \e end) for constant instances.
    *
    * In each row, given lambda function is performed. Each row is processed by at most one thread unlike the method
    * \ref DenseMatrixView::forElements where more than one thread can be mapped to each row.
    *
    * \tparam Function is type of the lambda function.
    *
    * \param begin defines beginning of the range [ \e begin,\e end ) of rows to be processed.
    * \param end defines ending of the range [ \e begin, \e end ) of rows to be processed.
    * \param function is an instance of the lambda function to be called for each row.
    *
    * ```
    * auto function = [] __cuda_callable__ ( RowView& row ) { ... };
    * ```
    *
    * \e RowView represents matrix row - see \ref TNL::Matrices::DenseMatrixView::RowView.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_forRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_forRows.out
    */
   template< typename Function >
   void
   forRows( IndexType begin, IndexType end, Function&& function ) const;

   /**
    * \brief Method for parallel iteration over all matrix rows.
    *
    * In each row, given lambda function is performed. Each row is processed by at most one thread unlike the method
    * \ref DenseMatrixView::forAllElements where more than one thread can be mapped to each row.
    *
    * \tparam Function is type of the lambda function.
    *
    * \param function is an instance of the lambda function to be called for each row.
    *
    * ```
    * auto function = [] __cuda_callable__ ( RowView& row ) mutable { ... };
    * ```
    *
    * \e RowView represents matrix row - see \ref TNL::Matrices::DenseMatrixView::RowView.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_forRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_forRows.out
    */
   template< typename Function >
   void
   forAllRows( Function&& function );

   /**
    * \brief Method for parallel iteration over all matrix rows for constant instances.
    *
    * In each row, given lambda function is performed. Each row is processed by at most one thread unlike the method
    * \ref DenseMatrixView::forAllElements where more than one thread can be mapped to each row.
    *
    * \tparam Function is type of the lambda function.
    *
    * \param function is an instance of the lambda function to be called for each row.
    *
    * ```
    * auto function = [] __cuda_callable__ ( RowView& row ) { ... };
    * ```
    *
    * \e RowView represents matrix row - see \ref TNL::Matrices::DenseMatrixView::RowView.
    *
    * \par Example
    * \include Matrices/DenseMatrix/DenseMatrixViewExample_forRows.cpp
    * \par Output
    * \include DenseMatrixViewExample_forRows.out
    */
   template< typename Function >
   void
   forAllRows( Function&& function ) const;

   /**
    * \brief Method for sequential iteration over all matrix rows for constant instances.
    *
    * \tparam Function is type of lambda function that will operate on matrix elements.
    *    It is should have form like
    *
    * ```
    * auto function = [] __cuda_callable__ ( RowView& row ) { ... };
    * ```
    *
    * \e RowView represents matrix row - see \ref TNL::Matrices::DenseMatrixView::RowView.
    *
    * \param begin defines beginning of the range [begin,end) of rows to be processed.
    * \param end defines ending of the range [begin,end) of rows to be processed.
    * \param function is an instance of the lambda function to be called in each row.
    */
   template< typename Function >
   void
   sequentialForRows( IndexType begin, IndexType end, Function&& function ) const;

   /**
    * \brief Method for sequential iteration over all matrix rows for non-constant instances.
    *
    * \tparam Function is type of lambda function that will operate on matrix elements.
    *    It is should have form like
    *
    * ```
    * auto function = [] __cuda_callable__ ( RowView& row ) { ... };
    * ```
    *
    * \e RowView represents matrix row - see \ref TNL::Matrices::DenseMatrixView::RowView.
    *
    * \param begin defines beginning of the range [begin,end) of rows to be processed.
    * \param end defines ending of the range [begin,end) of rows to be processed.
    * \param function is an instance of the lambda function to be called in each row.
    */
   template< typename Function >
   void
   sequentialForRows( IndexType begin, IndexType end, Function&& function );

   /**
    * \brief This method calls \e sequentialForRows for all matrix rows (for constant instances).
    *
    * See \ref DenseMatrixView::sequentialForRows.
    *
    * \tparam Function is a type of lambda function that will operate on matrix elements.
    * \param function  is an instance of the lambda function to be called in each row.
    */
   template< typename Function >
   void
   sequentialForAllRows( Function&& function ) const;

   /**
    * \brief This method calls \e sequentialForRows for all matrix rows.
    *
    * See \ref DenseMatrixView::sequentialForAllRows.
    *
    * \tparam Function is a type of lambda function that will operate on matrix elements.
    * \param function  is an instance of the lambda function to be called in each row.
    */
   template< typename Function >
   void
   sequentialForAllRows( Function&& function );

   /**
    * \brief Computes product of matrix and vector.
    *
    * More precisely, it computes:
    *
    * ```
    * outVector = matrixMultiplicator * ( *this ) * inVector + outVectorMultiplicator * outVector
    * ```
    *
    * \tparam InVector is type of input vector. It can be
    *         \ref TNL::Containers::Vector, \ref TNL::Containers::VectorView,
    *         \ref TNL::Containers::Array, \ref TNL::Containers::ArrayView,
    *         or similar container.
    * \tparam OutVector is type of output vector. It can be
    *         \ref TNL::Containers::Vector, \ref TNL::Containers::VectorView,
    *         \ref TNL::Containers::Array, \ref TNL::Containers::ArrayView,
    *         or similar container.
    *
    * \param inVector is input vector.
    * \param outVector is output vector.
    * \param matrixMultiplicator is a factor by which the matrix is multiplied. It is one by default.
    * \param outVectorMultiplicator is a factor by which the outVector is multiplied before added
    *    to the result of matrix-vector product. It is zero by default.
    * \param begin is the beginning of the rows range for which the vector product
    *    is computed. It is zero by default.
    * \param end is the end of the rows range for which the vector product
    *    is computed. It is number if the matrix rows by default.
    *
    * Note that the ouput vector dimension must be the same as the number of matrix rows
    * no matter how we set `begin` and `end` parameters. These parameters just say that
    * some matrix rows and the output vector elements are omitted.
    */
   template< typename InVector, typename OutVector >
   void
   vectorProduct( const InVector& inVector,
                  OutVector& outVector,
                  const RealType& matrixMultiplicator = 1.0,
                  const RealType& outVectorMultiplicator = 0.0,
                  IndexType begin = 0,
                  IndexType end = 0 ) const;

   template< typename Matrix >
   void
   addMatrix( const Matrix& matrix, const RealType& matrixMultiplicator = 1.0, const RealType& thisMatrixMultiplicator = 1.0 );

   /**
    * \brief Assignment operator with DenseMatrix.
    *
    * \param matrix is the right-hand side matrix.
    * \return reference to this matrix.
    */
   DenseMatrixView&
   operator=( const DenseMatrixView& matrix );

   /**
    * \brief Comparison operator with another dense matrix view.
    *
    * \param matrix is the right-hand side matrix view.
    * \return \e true if the RHS matrix view is equal, \e false otherwise.
    */
   template< typename Real_, typename Device_, typename Index_ >
   bool
   operator==( const DenseMatrixView< Real_, Device_, Index_, Organization >& matrix ) const;

   /**
    * \brief Comparison operator with another dense matrix view.
    *
    * \param matrix is the right-hand side matrix.
    * \return \e false if the RHS matrix view is equal, \e true otherwise.
    */
   template< typename Real_, typename Device_, typename Index_ >
   bool
   operator!=( const DenseMatrixView< Real_, Device_, Index_, Organization >& matrix ) const;

   /**
    * \brief Comparison operator with another arbitrary matrix type.
    *
    * \param matrix is the right-hand side matrix.
    * \return \e true if the RHS matrix is equal, \e false otherwise.
    */
   template< typename Matrix >
   bool
   operator==( const Matrix& matrix ) const;

   /**
    * \brief Comparison operator with another arbitrary matrix type.
    *
    * \param matrix is the right-hand side matrix.
    * \return \e true if the RHS matrix is equal, \e false otherwise.
    */
   template< typename Matrix >
   bool
   operator!=( const Matrix& matrix ) const;

   /**
    * \brief Method for saving the matrix view to the file with given filename.
    *
    * The ouput file can be loaded by \ref DenseMatrix.
    *
    * \param fileName is name of the file.
    */
   void
   save( const String& fileName ) const;

   /**
    * \brief Method for saving the matrix view to a file.
    *
    * The ouput file can be loaded by \ref DenseMatrix.
    *
    * \param file is the file where the matrix will be saved.
    */
   void
   save( File& file ) const override;

   /**
    * \brief Method for printing the matrix to output stream.
    *
    * \param str is the output stream.
    */
   void
   print( std::ostream& str ) const override;

protected:
   __cuda_callable__
   IndexType
   getElementIndex( IndexType row, IndexType column ) const;

   SegmentsViewType segments;
};

}  // namespace Matrices
}  // namespace noa::TNL

#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/DenseMatrixView.hpp>
