// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Assert.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/DenseMatrix.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Exceptions/NotImplementedError.h>

namespace noa::TNL {
namespace Matrices {

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::DenseMatrix( const RealAllocatorType& allocator )
: Matrix< Real, Device, Index, RealAllocator >( allocator )
{}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::DenseMatrix( const IndexType rows,
                                                                              const IndexType columns,
                                                                              const RealAllocatorType& allocator )
: Matrix< Real, Device, Index, RealAllocator >( allocator )
{
   this->setDimensions( rows, columns );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Value >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::DenseMatrix(
   std::initializer_list< std::initializer_list< Value > > data,
   const RealAllocatorType& allocator )
: Matrix< Real, Device, Index, RealAllocator >( allocator )
{
   this->setElements( data );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Value >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::setElements(
   std::initializer_list< std::initializer_list< Value > > data )
{
   IndexType rows = data.size();
   IndexType columns = 0;
   for( auto row : data )
      columns = max( columns, row.size() );
   this->setDimensions( rows, columns );
   if constexpr( std::is_same< DeviceType, Devices::Cuda >::value ) {
      DenseMatrix< RealType, Devices::Host, IndexType > hostDense( rows, columns );
      IndexType rowIdx( 0 );
      for( auto row : data ) {
         IndexType columnIdx( 0 );
         for( auto element : row )
            hostDense.setElement( rowIdx, columnIdx++, element );
         rowIdx++;
      }
      *this = hostDense;
   }
   else {
      IndexType rowIdx( 0 );
      for( auto row : data ) {
         IndexType columnIdx( 0 );
         for( auto element : row )
            this->setElement( rowIdx, columnIdx++, element );
         rowIdx++;
      }
   }
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
auto
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getView() -> ViewType
{
   ValuesView values_view = this->getValues().getView();
   // note this is improtant here to avoid const qualifier to appear in - somehow :(
   return ViewType( this->getRows(), this->getColumns(), values_view );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
auto
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getConstView() const -> ConstViewType
{
   return ConstViewType( this->getRows(), this->getColumns(), this->getValues().getConstView() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
std::string
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getSerializationType()
{
   return ViewType::getSerializationType();
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
std::string
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getSerializationTypeVirtual() const
{
   return this->getSerializationType();
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::setDimensions( const IndexType rows, const IndexType columns )
{
   Matrix< Real, Device, Index, RealAllocator >::setDimensions( rows, columns );
   this->segments.setSegmentsSizes( rows, columns );
   this->values.setSize( this->segments.getStorageSize() );
   this->values = 0.0;
   this->view = this->getView();
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Matrix_ >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::setLike( const Matrix_& matrix )
{
   this->setDimensions( matrix.getRows(), matrix.getColumns() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RowCapacitiesVector >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::setRowCapacities( const RowCapacitiesVector& rowCapacities )
{
   TNL_ASSERT_EQ( rowCapacities.getSize(), this->getRows(), "" );
   TNL_ASSERT_LE( max( rowCapacities ), this->getColumns(), "" );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Vector >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getRowCapacities( Vector& rowCapacities ) const
{
   this->view.getCompressedRowLengths( rowCapacities );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RowLengthsVector >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getCompressedRowLengths( RowLengthsVector& rowLengths ) const
{
   this->view.getCompressedRowLengths( rowLengths );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
Index
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getNonzeroElementsCount() const
{
   return this->view.getNonzeroElementsCount();
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::reset()
{
   Matrix< Real, Device, Index >::reset();
   this->segments.reset();
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::setValue( const Real& value )
{
   this->view.setValue( value );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
auto
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getRow( const IndexType& rowIdx ) const -> ConstRowView
{
   return this->view.getRow( rowIdx );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
auto
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getRow( const IndexType& rowIdx ) -> RowView
{
   return this->view.getRow( rowIdx );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
Real&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator()( const IndexType row, const IndexType column )
{
   return this->view.operator()( row, column );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
const Real&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator()( const IndexType row, const IndexType column ) const
{
   return this->view.operator()( row, column );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::setElement( const IndexType row,
                                                                             const IndexType column,
                                                                             const RealType& value )
{
   this->view.setElement( row, column, value );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::addElement( const IndexType row,
                                                                             const IndexType column,
                                                                             const RealType& value,
                                                                             const RealType& thisElementMultiplicator )
{
   this->view.addElement( row, column, value, thisElementMultiplicator );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
Real
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getElement( const IndexType row, const IndexType column ) const
{
   return this->view.getElement( row, column );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Fetch, typename Reduce, typename Keep, typename FetchValue >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::reduceRows( IndexType begin,
                                                                             IndexType end,
                                                                             Fetch& fetch,
                                                                             const Reduce& reduce,
                                                                             Keep& keep,
                                                                             const FetchValue& identity )
{
   this->view.reduceRows( begin, end, fetch, reduce, keep, identity );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Fetch, typename Reduce, typename Keep, typename FetchValue >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::reduceRows( IndexType begin,
                                                                             IndexType end,
                                                                             Fetch& fetch,
                                                                             const Reduce& reduce,
                                                                             Keep& keep,
                                                                             const FetchValue& identity ) const
{
   this->view.reduceRows( begin, end, fetch, reduce, keep, identity );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Fetch, typename Reduce, typename Keep, typename FetchReal >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::reduceAllRows( Fetch& fetch,
                                                                                const Reduce& reduce,
                                                                                Keep& keep,
                                                                                const FetchReal& identity )
{
   this->reduceRows( (IndexType) 0, this->getRows(), fetch, reduce, keep, identity );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Fetch, typename Reduce, typename Keep, typename FetchReal >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::reduceAllRows( Fetch& fetch,
                                                                                const Reduce& reduce,
                                                                                Keep& keep,
                                                                                const FetchReal& identity ) const
{
   this->reduceRows( (IndexType) 0, this->getRows(), fetch, reduce, keep, identity );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forElements( IndexType begin,
                                                                              IndexType end,
                                                                              Function&& function ) const
{
   this->view.forElements( begin, end, function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forElements( IndexType begin,
                                                                              IndexType end,
                                                                              Function&& function )
{
   this->view.forElements( begin, end, function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forAllElements( Function&& function ) const
{
   this->forElements( (IndexType) 0, this->getRows(), function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forAllElements( Function&& function )
{
   this->forElements( (IndexType) 0, this->getRows(), function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forRows( IndexType begin, IndexType end, Function&& function )
{
   this->getView().forRows( begin, end, function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forRows( IndexType begin,
                                                                          IndexType end,
                                                                          Function&& function ) const
{
   this->getConstView().forRows( begin, end, function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forAllRows( Function&& function )
{
   this->getView().forAllRows( function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::forAllRows( Function&& function ) const
{
   this->getConsView().forAllRows( function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::sequentialForRows( IndexType begin,
                                                                                    IndexType end,
                                                                                    Function&& function ) const
{
   this->view.sequentialForRows( begin, end, function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::sequentialForRows( IndexType begin,
                                                                                    IndexType end,
                                                                                    Function&& function )
{
   this->view.sequentialForRows( begin, end, function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::sequentialForAllRows( Function&& function ) const
{
   this->sequentialForRows( (IndexType) 0, this->getRows(), function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Function >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::sequentialForAllRows( Function&& function )
{
   this->sequentialForRows( (IndexType) 0, this->getRows(), function );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename InVector, typename OutVector >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::vectorProduct( const InVector& inVector,
                                                                                OutVector& outVector,
                                                                                const RealType& matrixMultiplicator,
                                                                                const RealType& outVectorMultiplicator,
                                                                                const IndexType begin,
                                                                                const IndexType end ) const
{
   this->view.vectorProduct( inVector, outVector, matrixMultiplicator, outVectorMultiplicator, begin, end );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Matrix >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::addMatrix( const Matrix& matrix,
                                                                            const RealType& matrixMultiplicator,
                                                                            const RealType& thisMatrixMultiplicator )
{
   TNL_ASSERT( this->getColumns() == matrix.getColumns() && this->getRows() == matrix.getRows(),
               std::cerr << "This matrix columns: " << this->getColumns() << std::endl
                         << "This matrix rows: " << this->getRows() << std::endl
                         << "That matrix columns: " << matrix.getColumns() << std::endl
                         << "That matrix rows: " << matrix.getRows() << std::endl );

   if( thisMatrixMultiplicator == 1.0 )
      this->values += matrixMultiplicator * matrix.values;
   else
      this->values = thisMatrixMultiplicator * this->values + matrixMultiplicator * matrix.values;
}

template< int tileDim, int tileRowBlockSize, typename ResultMatrix, typename Matrix1, typename Matrix2 >
__global__
void
DenseMatrixProductKernel( ResultMatrix resultMatrix,
                          const Matrix1 matrixA,
                          const Matrix2 matrixB,
                          const typename ResultMatrix::RealType matrixMultiplicator,
                          const typename ResultMatrix::IndexType gridIdx_x,
                          const typename ResultMatrix::IndexType gridIdx_y )
{
#ifdef __CUDACC__
   /****
    * Here we compute product C = A * B. To profit from the fast
    * shared memory we do it by tiles.
    */

   using IndexType = typename ResultMatrix::IndexType;
   using RealType = typename ResultMatrix::RealType;
   __shared__ RealType tileA[ tileDim * tileDim ];
   __shared__ RealType tileB[ tileDim * tileDim ];
   __shared__ RealType tileC[ tileDim * tileDim ];

   const IndexType& matrixARows = matrixA.getRows();
   const IndexType& matrixAColumns = matrixA.getColumns();
   const IndexType& matrixBRows = matrixB.getRows();
   const IndexType& matrixBColumns = matrixB.getColumns();

   /****
    * Reset the tile C
    */
   for( IndexType row = 0; row < tileDim; row += tileRowBlockSize )
      tileC[ ( row + threadIdx.y ) * tileDim + threadIdx.x ] = 0.0;

   /****
    * Compute the result tile coordinates
    */
   const IndexType resultTileRow = ( gridIdx_y * gridDim.y + blockIdx.y ) * tileDim;
   const IndexType resultTileColumn = ( gridIdx_x * gridDim.x + blockIdx.x ) * tileDim;

   /****
    * Sum over the matrix tiles
    */
   for( IndexType i = 0; i < matrixAColumns; i += tileDim ) {
      for( IndexType row = 0; row < tileDim; row += tileRowBlockSize ) {
         const IndexType matrixARow = resultTileRow + threadIdx.y + row;
         const IndexType matrixAColumn = i + threadIdx.x;
         if( matrixARow < matrixARows && matrixAColumn < matrixAColumns )
            tileA[ ( threadIdx.y + row ) * tileDim + threadIdx.x ] = matrixA( matrixARow, matrixAColumn );

         const IndexType matrixBRow = i + threadIdx.y + row;
         const IndexType matrixBColumn = resultTileColumn + threadIdx.x;
         if( matrixBRow < matrixBRows && matrixBColumn < matrixBColumns )
            tileB[ ( threadIdx.y + row ) * tileDim + threadIdx.x ] = matrixB( matrixBRow, matrixBColumn );
      }
      __syncthreads();

      const IndexType tileALastRow = TNL::min( tileDim, matrixARows - resultTileRow );
      const IndexType tileALastColumn = TNL::min( tileDim, matrixAColumns - i );
      // const IndexType tileBLastRow = TNL::min( tileDim, matrixBRows - i );
      // const IndexType tileBLastColumn = TNL::min( tileDim, matrixBColumns - resultTileColumn );

      for( IndexType row = 0; row < tileALastRow; row += tileRowBlockSize ) {
         RealType sum( 0.0 );
         for( IndexType j = 0; j < tileALastColumn; j++ )
            sum += matrixMultiplicator * tileA[ ( threadIdx.y + row ) * tileDim + j ] * tileB[ j * tileDim + threadIdx.x ];
         tileC[ ( row + threadIdx.y ) * tileDim + threadIdx.x ] += sum;
      }
      __syncthreads();
   }

   /****
    * Write the result tile to the result matrix
    */
   const IndexType& matrixCRows = resultMatrix.getRows();
   const IndexType& matrixCColumns = resultMatrix.getColumns();
   for( IndexType row = 0; row < tileDim; row += tileRowBlockSize ) {
      const IndexType matrixCRow = resultTileRow + row + threadIdx.y;
      const IndexType matrixCColumn = resultTileColumn + threadIdx.x;
      if( matrixCRow < matrixCRows && matrixCColumn < matrixCColumns )
         resultMatrix( matrixCRow, matrixCColumn ) = tileC[ ( row + threadIdx.y ) * tileDim + threadIdx.x ];
   }
#endif
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Matrix1, typename Matrix2, int tileDim >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getMatrixProduct( const Matrix1& matrix1,
                                                                                   const Matrix2& matrix2,
                                                                                   const RealType& matrixMultiplicator )
{
   TNL_ASSERT_EQ( matrix1.getColumns(), matrix2.getRows(), "invalid dimensions of input matrices" );
   setDimensions( matrix1.getRows(), matrix2.getColumns() );

   if constexpr( std::is_same< Device, Devices::Host >::value )
      for( IndexType i = 0; i < this->getRows(); i += tileDim )
         for( IndexType j = 0; j < this->getColumns(); j += tileDim ) {
            const IndexType tileRows = min( tileDim, this->getRows() - i );
            const IndexType tileColumns = min( tileDim, this->getColumns() - j );
            for( IndexType i1 = i; i1 < i + tileRows; i1++ )
               for( IndexType j1 = j; j1 < j + tileColumns; j1++ )
                  operator()( i1, j1 ) = 0.0;

            for( IndexType k = 0; k < matrix1.getColumns(); k += tileDim ) {
               const IndexType lastK = min( k + tileDim, matrix1.getColumns() );
               for( IndexType i1 = 0; i1 < tileRows; i1++ )
                  for( IndexType j1 = 0; j1 < tileColumns; j1++ )
                     for( IndexType k1 = k; k1 < lastK; k1++ )
                        operator()( i + i1, j + j1 ) += matrixMultiplicator * matrix1( i + i1, k1 ) * matrix2( k1, j + j1 );
            }
         }
   if constexpr( std::is_same< Device, Devices::Cuda >::value ) {
      constexpr IndexType matrixProductCudaBlockSize = 256;
      constexpr IndexType cudaBlockRows = matrixProductCudaBlockSize / tileDim;
      Cuda::LaunchConfiguration launch_config;
      launch_config.blockSize.x = tileDim;
      launch_config.blockSize.y = cudaBlockRows;
      launch_config.dynamicSharedMemorySize = 3 * tileDim * tileDim;

      const IndexType rowTiles = roundUpDivision( this->getRows(), tileDim );
      const IndexType columnTiles = roundUpDivision( this->getColumns(), tileDim );
      const IndexType rowGrids = roundUpDivision( rowTiles, Cuda::getMaxGridYSize() );
      const IndexType columnGrids = roundUpDivision( columnTiles, Cuda::getMaxGridXSize() );

      for( IndexType gridIdx_x = 0; gridIdx_x < columnGrids; gridIdx_x++ )
         for( IndexType gridIdx_y = 0; gridIdx_y < rowGrids; gridIdx_y++ ) {
            launch_config.gridSize.x = Cuda::getMaxGridXSize();
            launch_config.gridSize.y = Cuda::getMaxGridYSize();
            if( gridIdx_x == columnGrids - 1 )
               launch_config.gridSize.x = columnTiles % Cuda::getMaxGridXSize();
            if( gridIdx_y == rowGrids - 1 )
               launch_config.gridSize.y = rowTiles % Cuda::getMaxGridYSize();

            constexpr auto kernel = DenseMatrixProductKernel< tileDim,
                                                              cudaBlockRows,
                                                              ViewType,
                                                              typename Matrix1::ConstViewType,
                                                              typename Matrix2::ConstViewType >;
            Cuda::launchKernelAsync( kernel,
                                     launch_config,
                                     getView(),
                                     matrix1.getConstView(),
                                     matrix2.getConstView(),
                                     matrixMultiplicator,
                                     gridIdx_x,
                                     gridIdx_y );
         }
      cudaStreamSynchronize( launch_config.stream );
      TNL_CHECK_CUDA_DEVICE;
   }
}

template< int tileDim, int tileRowBlockSize, typename OutputMatrix, typename InputMatrix, typename Real, typename Index >
__global__
void
DenseTranspositionAlignedKernel( OutputMatrix resultMatrix,
                                 const InputMatrix inputMatrix,
                                 const Real matrixMultiplicator,
                                 const Index gridIdx_x,
                                 const Index gridIdx_y )
{
#ifdef __CUDACC__
   __shared__ Real tile[ tileDim * tileDim ];

   const Index columns = inputMatrix.getColumns();
   const Index rows = inputMatrix.getRows();

   /****
    * Diagonal mapping of the CUDA blocks
    */
   Index blockIdx_x, blockIdx_y;
   if( columns == rows ) {
      blockIdx_y = blockIdx.x;
      blockIdx_x = ( blockIdx.x + blockIdx.y ) % gridDim.x;
   }
   else {
      Index bID = blockIdx.x + gridDim.x * blockIdx.y;
      blockIdx_y = bID % gridDim.y;
      blockIdx_x = ( ( bID / gridDim.y ) + blockIdx_y ) % gridDim.x;
   }

   /****
    * Read the tile to the shared memory
    */
   const Index readRowPosition = ( gridIdx_y * gridDim.y + blockIdx_y ) * tileDim + threadIdx.y;
   const Index readColumnPosition = ( gridIdx_x * gridDim.x + blockIdx_x ) * tileDim + threadIdx.x;
   for( Index rowBlock = 0; rowBlock < tileDim; rowBlock += tileRowBlockSize ) {
      tile[ Cuda::getInterleaving( threadIdx.x * tileDim + threadIdx.y + rowBlock ) ] =
         inputMatrix( readRowPosition + rowBlock, readColumnPosition );
   }
   __syncthreads();

   /****
    * Write the tile to the global memory
    */
   const Index writeRowPosition = ( gridIdx_x * gridDim.x + blockIdx_x ) * tileDim + threadIdx.y;
   const Index writeColumnPosition = ( gridIdx_y * gridDim.y + blockIdx_y ) * tileDim + threadIdx.x;
   for( Index rowBlock = 0; rowBlock < tileDim; rowBlock += tileRowBlockSize ) {
      resultMatrix( writeRowPosition + rowBlock, writeColumnPosition ) =
         matrixMultiplicator * tile[ Cuda::getInterleaving( ( threadIdx.y + rowBlock ) * tileDim + threadIdx.x ) ];
   }
#endif
}

template< int tileDim, int tileRowBlockSize, typename OutputMatrix, typename InputMatrix, typename Real, typename Index >
__global__
void
DenseTranspositionNonAlignedKernel( OutputMatrix resultMatrix,
                                    const InputMatrix inputMatrix,
                                    const Real matrixMultiplicator,
                                    const Index gridIdx_x,
                                    const Index gridIdx_y )
{
#ifdef __CUDACC__
   __shared__ Real tile[ tileDim * tileDim ];

   const Index columns = inputMatrix.getColumns();
   const Index rows = inputMatrix.getRows();

   /****
    * Diagonal mapping of the CUDA blocks
    */
   Index blockIdx_x, blockIdx_y;
   if( columns == rows ) {
      blockIdx_y = blockIdx.x;
      blockIdx_x = ( blockIdx.x + blockIdx.y ) % gridDim.x;
   }
   else {
      Index bID = blockIdx.x + gridDim.x * blockIdx.y;
      blockIdx_y = bID % gridDim.y;
      blockIdx_x = ( ( bID / gridDim.y ) + blockIdx_y ) % gridDim.x;
   }

   /****
    * Read the tile to the shared memory
    */
   const Index readRowPosition = ( gridIdx_y * gridDim.y + blockIdx_y ) * tileDim + threadIdx.y;
   const Index readColumnPosition = ( gridIdx_x * gridDim.x + blockIdx_x ) * tileDim + threadIdx.x;
   if( readColumnPosition < columns ) {
      // const Index readOffset = readRowPosition * columns + readColumnPosition;
      for( Index rowBlock = 0; rowBlock < tileDim; rowBlock += tileRowBlockSize ) {
         if( readRowPosition + rowBlock < rows )
            tile[ Cuda::getInterleaving( threadIdx.x * tileDim + threadIdx.y + rowBlock ) ] =
               inputMatrix( readRowPosition + rowBlock, readColumnPosition );
      }
   }
   __syncthreads();

   /****
    * Write the tile to the global memory
    */
   const Index writeRowPosition = ( gridIdx_x * gridDim.x + blockIdx_x ) * tileDim + threadIdx.y;
   const Index writeColumnPosition = ( gridIdx_y * gridDim.y + blockIdx_y ) * tileDim + threadIdx.x;
   if( writeColumnPosition < rows ) {
      // const Index writeOffset = writeRowPosition * rows + writeColumnPosition;
      for( Index rowBlock = 0; rowBlock < tileDim; rowBlock += tileRowBlockSize ) {
         if( writeRowPosition + rowBlock < columns )
            resultMatrix( writeRowPosition + rowBlock, writeColumnPosition ) =
               matrixMultiplicator * tile[ Cuda::getInterleaving( ( threadIdx.y + rowBlock ) * tileDim + threadIdx.x ) ];
      }
   }
#endif
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Matrix, int tileDim >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getTransposition( const Matrix& matrix,
                                                                                   const RealType& matrixMultiplicator )
{
   setDimensions( matrix.getColumns(), matrix.getRows() );

   if constexpr( std::is_same< Device, Devices::Host >::value ) {
      const IndexType& rows = matrix.getRows();
      const IndexType& columns = matrix.getColumns();
      for( IndexType i = 0; i < rows; i += tileDim )
         for( IndexType j = 0; j < columns; j += tileDim )
            for( IndexType k = i; k < i + tileDim && k < rows; k++ )
               for( IndexType l = j; l < j + tileDim && l < columns; l++ )
                  this->setElement( l, k, matrixMultiplicator * matrix.getElement( k, l ) );
   }
   if constexpr( std::is_same< Device, Devices::Cuda >::value ) {
      constexpr IndexType matrixProductCudaBlockSize = 256;
      constexpr IndexType cudaBlockRows = matrixProductCudaBlockSize / tileDim;
      Cuda::LaunchConfiguration launch_config;
      launch_config.blockSize.x = tileDim;
      launch_config.blockSize.y = cudaBlockRows;
      launch_config.dynamicSharedMemorySize = tileDim * tileDim + tileDim * tileDim / Cuda::getNumberOfSharedMemoryBanks();

      const IndexType rowTiles = roundUpDivision( this->getRows(), tileDim );
      const IndexType columnTiles = roundUpDivision( this->getColumns(), tileDim );
      const IndexType rowGrids = roundUpDivision( rowTiles, Cuda::getMaxGridYSize() );
      const IndexType columnGrids = roundUpDivision( columnTiles, Cuda::getMaxGridXSize() );

      for( IndexType gridIdx_x = 0; gridIdx_x < columnGrids; gridIdx_x++ )
         for( IndexType gridIdx_y = 0; gridIdx_y < rowGrids; gridIdx_y++ ) {
            launch_config.gridSize.x = Cuda::getMaxGridXSize();
            launch_config.gridSize.y = Cuda::getMaxGridYSize();
            if( gridIdx_x == columnGrids - 1 )
               launch_config.gridSize.x = columnTiles % Cuda::getMaxGridXSize();
            if( gridIdx_y == rowGrids - 1 )
               launch_config.gridSize.y = rowTiles % Cuda::getMaxGridYSize();

            if( ( gridIdx_x < columnGrids - 1 || matrix.getColumns() % tileDim == 0 )
                && ( gridIdx_y < rowGrids - 1 || matrix.getRows() % tileDim == 0 ) )
            {
               constexpr auto kernel = DenseTranspositionAlignedKernel< tileDim,
                                                                        cudaBlockRows,
                                                                        ViewType,
                                                                        typename Matrix::ConstViewType,
                                                                        RealType,
                                                                        IndexType >;
               Cuda::launchKernelAsync(
                  kernel, launch_config, getView(), matrix.getConstView(), matrixMultiplicator, gridIdx_x, gridIdx_y );
            }
            else {
               constexpr auto kernel = DenseTranspositionNonAlignedKernel< tileDim,
                                                                           cudaBlockRows,
                                                                           ViewType,
                                                                           typename Matrix::ConstViewType,
                                                                           RealType,
                                                                           IndexType >;
               Cuda::launchKernelAsync(
                  kernel, launch_config, getView(), matrix.getConstView(), matrixMultiplicator, gridIdx_x, gridIdx_y );
            }
         }
      cudaStreamSynchronize( launch_config.stream );
      TNL_CHECK_CUDA_DEVICE;
   }
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator=(
   const DenseMatrix< Real, Device, Index, Organization, RealAllocator >& matrix )
{
   return this->operator=( matrix.getConstView() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RHSReal, typename RHSDevice, typename RHSIndex, typename RHSRealAllocator >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator=(
   const DenseMatrix< RHSReal, RHSDevice, RHSIndex, Organization, RHSRealAllocator >& matrix )
{
   return this->operator=( matrix.getConstView() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RHSReal, typename RHSDevice, typename RHSIndex >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator=(
   const DenseMatrixView< RHSReal, RHSDevice, RHSIndex, Organization >& matrix )
{
   this->setLike( matrix );
   this->values = matrix.getValues();
   return *this;
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RHSReal,
          typename RHSDevice,
          typename RHSIndex,
          ElementsOrganization RHSOrganization,
          typename RHSRealAllocator >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator=(
   const DenseMatrix< RHSReal, RHSDevice, RHSIndex, RHSOrganization, RHSRealAllocator >& matrix )
{
   return this->operator=( matrix.getConstView() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RHSReal, typename RHSDevice, typename RHSIndex, ElementsOrganization RHSOrganization >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator=(
   const DenseMatrixView< RHSReal, RHSDevice, RHSIndex, RHSOrganization >& matrix )
{
   using RHSMatrix = DenseMatrixView< RHSReal, RHSDevice, RHSIndex, RHSOrganization >;
   using RHSIndexType = typename RHSMatrix::IndexType;
   using RHSRealType = std::remove_const_t< typename RHSMatrix::RealType >;
   using RHSDeviceType = typename RHSMatrix::DeviceType;

   this->setLike( matrix );
   if( Organization == RHSOrganization ) {
      this->values = matrix.getValues();
      return *this;
   }

   auto this_view = this->view;
   if constexpr( std::is_same< DeviceType, RHSDeviceType >::value ) {
      auto f = [ = ] __cuda_callable__(
                  RHSIndexType rowIdx, RHSIndexType localIdx, RHSIndexType columnIdx, const RHSRealType& value ) mutable
      {
         this_view( rowIdx, columnIdx ) = value;
      };
      matrix.forAllElements( f );
   }
   else {
      const IndexType maxRowLength = matrix.getColumns();
      const IndexType bufferRowsCount( 128 );
      const size_t bufferSize = bufferRowsCount * maxRowLength;
      Containers::Vector< RHSRealType, RHSDeviceType, RHSIndexType > matrixValuesBuffer( bufferSize );
      Containers::Vector< RealType, DeviceType, IndexType > thisValuesBuffer( bufferSize );
      auto matrixValuesBuffer_view = matrixValuesBuffer.getView();
      auto thisValuesBuffer_view = thisValuesBuffer.getView();

      IndexType baseRow( 0 );
      const IndexType rowsCount = this->getRows();
      while( baseRow < rowsCount ) {
         const IndexType lastRow = min( baseRow + bufferRowsCount, rowsCount );

         ////
         // Copy matrix elements into buffer
         auto f1 = [ = ] __cuda_callable__(
                      RHSIndexType rowIdx, RHSIndexType localIdx, RHSIndexType columnIdx, const RHSRealType& value ) mutable
         {
            const IndexType bufferIdx = ( rowIdx - baseRow ) * maxRowLength + columnIdx;
            matrixValuesBuffer_view[ bufferIdx ] = value;
         };
         matrix.forElements( baseRow, lastRow, f1 );

         ////
         // Copy the source matrix buffer to this matrix buffer
         thisValuesBuffer_view = matrixValuesBuffer_view;

         ////
         // Copy matrix elements from the buffer to the matrix.
         auto this_view = this->view;
         auto f2 = [ = ] __cuda_callable__( IndexType columnIdx, IndexType bufferRowIdx ) mutable
         {
            IndexType bufferIdx = bufferRowIdx * maxRowLength + columnIdx;
            this_view( baseRow + bufferRowIdx, columnIdx ) = thisValuesBuffer_view[ bufferIdx ];
         };
         Algorithms::ParallelFor2D< DeviceType >::exec(
            (IndexType) 0, (IndexType) 0, maxRowLength, (IndexType) min( bufferRowsCount, this->getRows() - baseRow ), f2 );
         baseRow += bufferRowsCount;
      }
   }
   return *this;
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename RHSMatrix >
DenseMatrix< Real, Device, Index, Organization, RealAllocator >&
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator=( const RHSMatrix& matrix )
{
   using RHSIndexType = typename RHSMatrix::IndexType;
   using RHSRealType = typename RHSMatrix::RealType;
   using RHSDeviceType = typename RHSMatrix::DeviceType;
   using RHSRealAllocatorType = typename RHSMatrix::RealAllocatorType;

   Containers::Vector< RHSIndexType, RHSDeviceType, RHSIndexType > rowLengths;
   matrix.getCompressedRowLengths( rowLengths );
   this->setDimensions( matrix.getRows(), matrix.getColumns() );

   auto values_view = this->values.getView();
   RHSIndexType padding_index = matrix.getPaddingIndex();
   this->values = 0.0;

   if constexpr( std::is_same< DeviceType, RHSDeviceType >::value ) {
      const auto segments_view = this->segments.getView();
      auto f = [ = ] __cuda_callable__(
                  RHSIndexType rowIdx, RHSIndexType localIdx_, RHSIndexType columnIdx, const RHSRealType& value ) mutable
      {
         if( value != 0.0 && columnIdx != padding_index )
            values_view[ segments_view.getGlobalIndex( rowIdx, columnIdx ) ] = value;
      };
      matrix.forAllElements( f );
   }
   else {
      const IndexType maxRowLength = max( rowLengths );
      const IndexType bufferRowsCount( 128 );
      const size_t bufferSize = bufferRowsCount * maxRowLength;
      Containers::Vector< RHSRealType, RHSDeviceType, RHSIndexType, RHSRealAllocatorType > matrixValuesBuffer( bufferSize );
      Containers::Vector< RHSIndexType, RHSDeviceType, RHSIndexType > matrixColumnsBuffer( bufferSize );
      Containers::Vector< RealType, DeviceType, IndexType, RealAllocatorType > thisValuesBuffer( bufferSize );
      Containers::Vector< IndexType, DeviceType, IndexType > thisColumnsBuffer( bufferSize );
      auto matrixValuesBuffer_view = matrixValuesBuffer.getView();
      auto matrixColumnsBuffer_view = matrixColumnsBuffer.getView();
      auto thisValuesBuffer_view = thisValuesBuffer.getView();
      auto thisColumnsBuffer_view = thisColumnsBuffer.getView();

      IndexType baseRow( 0 );
      const IndexType rowsCount = this->getRows();
      while( baseRow < rowsCount ) {
         const IndexType lastRow = min( baseRow + bufferRowsCount, rowsCount );
         thisColumnsBuffer = padding_index;
         matrixColumnsBuffer_view = padding_index;

         ////
         // Copy matrix elements into buffer
         auto f1 = [ = ] __cuda_callable__(
                      RHSIndexType rowIdx, RHSIndexType localIdx, RHSIndexType columnIndex, const RHSRealType& value ) mutable
         {
            if( columnIndex != padding_index ) {
               const IndexType bufferIdx = ( rowIdx - baseRow ) * maxRowLength + localIdx;
               matrixColumnsBuffer_view[ bufferIdx ] = columnIndex;
               matrixValuesBuffer_view[ bufferIdx ] = value;
            }
         };
         matrix.forElements( baseRow, lastRow, f1 );

         ////
         // Copy the source matrix buffer to this matrix buffer
         thisValuesBuffer_view = matrixValuesBuffer_view;
         thisColumnsBuffer_view = matrixColumnsBuffer_view;

         ////
         // Copy matrix elements from the buffer to the matrix
         auto this_view = this->view;
         auto f2 = [ = ] __cuda_callable__( IndexType bufferColumnIdx, IndexType bufferRowIdx ) mutable
         {
            IndexType bufferIdx = bufferRowIdx * maxRowLength + bufferColumnIdx;
            IndexType columnIdx = thisColumnsBuffer_view[ bufferIdx ];
            if( columnIdx != padding_index )
               this_view( baseRow + bufferRowIdx, columnIdx ) = thisValuesBuffer_view[ bufferIdx ];
         };
         Algorithms::ParallelFor2D< DeviceType >::exec(
            (IndexType) 0, (IndexType) 0, maxRowLength, (IndexType) min( bufferRowsCount, this->getRows() - baseRow ), f2 );
         baseRow += bufferRowsCount;
      }
   }
   this->view = this->getView();
   return *this;
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Real_, typename Device_, typename Index_, typename RealAllocator_ >
bool
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator==(
   const DenseMatrix< Real_, Device_, Index_, Organization, RealAllocator_ >& matrix ) const
{
   return ( this->getRows() == matrix.getRows() && this->getColumns() == matrix.getColumns()
            && this->getValues() == matrix.getValues() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Real_, typename Device_, typename Index_, typename RealAllocator_ >
bool
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator!=(
   const DenseMatrix< Real_, Device_, Index_, Organization, RealAllocator_ >& matrix ) const
{
   return ! ( *this == matrix );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Real_, typename Device_, typename Index_ >
bool
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator==(
   const DenseMatrixView< Real_, Device_, Index_, Organization >& matrix ) const
{
   return ( this->getRows() == matrix.getRows() && this->getColumns() == matrix.getColumns()
            && this->getValues() == matrix.getValues() );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Real_, typename Device_, typename Index_ >
bool
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator!=(
   const DenseMatrixView< Real_, Device_, Index_, Organization >& matrix ) const
{
   return ! ( *this == matrix );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Matrix >
bool
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator==( const Matrix& m ) const
{
   return ( this->view == m );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
template< typename Matrix >
bool
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::operator!=( const Matrix& m ) const
{
   return ( this->view != m );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::save( const String& fileName ) const
{
   Object::save( fileName );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::load( const String& fileName )
{
   Object::load( fileName );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::save( File& file ) const
{
   this->view.save( file );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::load( File& file )
{
   Matrix< Real, Device, Index, RealAllocator >::load( file );
   this->segments.load( file );
   this->view = this->getView();
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
void
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::print( std::ostream& str ) const
{
   this->view.print( str );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
__cuda_callable__
Index
DenseMatrix< Real, Device, Index, Organization, RealAllocator >::getElementIndex( const IndexType row,
                                                                                  const IndexType column ) const
{
   return this->segments.getGlobalIndex( row, column );
}

template< typename Real, typename Device, typename Index, ElementsOrganization Organization, typename RealAllocator >
std::ostream&
operator<<( std::ostream& str, const DenseMatrix< Real, Device, Index, Organization, RealAllocator >& matrix )
{
   matrix.print( str );
   return str;
}

template< typename Real,
          typename Device,
          typename Index,
          typename Real_,
          typename Device_,
          typename Index_,
          ElementsOrganization Organization,
          typename RealAllocator >
bool
operator==( const DenseMatrixView< Real, Device, Index, Organization >& leftMatrix,
            const DenseMatrix< Real_, Device_, Index_, Organization, RealAllocator >& rightMatrix )
{
   return rightMatrix == leftMatrix;
}

template< typename Real,
          typename Device,
          typename Index,
          typename Real_,
          typename Device_,
          typename Index_,
          ElementsOrganization Organization,
          typename RealAllocator >
bool
operator!=( const DenseMatrixView< Real, Device, Index, Organization >& leftMatrix,
            const DenseMatrix< Real_, Device_, Index_, Organization, RealAllocator >& rightMatrix )
{
   return rightMatrix != leftMatrix;
}

}  // namespace Matrices
}  // namespace noa::TNL
