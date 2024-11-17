// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Containers/Vector.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/ParallelFor.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Segments/CSRView.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Segments/detail/CSR.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Segments/detail/LambdaAdapter.h>

namespace noa::TNL {
namespace Algorithms {
namespace Segments {

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
CSRView< Device, Index, Kernel >::CSRView( const OffsetsView& offsets, const KernelView& kernel )
: offsets( offsets ), kernel( kernel )
{}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
CSRView< Device, Index, Kernel >::CSRView( OffsetsView&& offsets, KernelView&& kernel )
: offsets( std::move( offsets ) ), kernel( std::move( kernel ) )
{}

template< typename Device, typename Index, typename Kernel >
template< typename Index2 >
__cuda_callable__
CSRView< Device, Index, Kernel >::CSRView( const CSRView< Device, Index2, Kernel >& csr_view )
: offsets( csr_view.getOffsets() ), kernel( csr_view.getKernel() )
{}

template< typename Device, typename Index, typename Kernel >
std::string
CSRView< Device, Index, Kernel >::getSerializationType()
{
   return "CSR< [any_device], " + TNL::getSerializationType< IndexType >() + ", " +
          // FIXME: the serialized data do not depend on the the kernel type so it should not be in the serialization type
          TNL::getSerializationType< KernelType >() + " >";
}

template< typename Device, typename Index, typename Kernel >
String
CSRView< Device, Index, Kernel >::getSegmentsType()
{
   return "CSR< " + KernelType::getKernelType() + " >";
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
typename CSRView< Device, Index, Kernel >::ViewType
CSRView< Device, Index, Kernel >::getView()
{
   return { this->offsets, this->kernel };
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getConstView() const -> ConstViewType
{
   return { this->offsets.getConstView(), this->kernel.getConstView() };
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getSegmentsCount() const -> IndexType
{
   return this->offsets.getSize() - 1;
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getSegmentSize( const IndexType segmentIdx ) const -> IndexType
{
   return detail::CSR< Device, Index >::getSegmentSize( this->offsets, segmentIdx );
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getSize() const -> IndexType
{
   return this->getStorageSize();
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getStorageSize() const -> IndexType
{
   return detail::CSR< Device, Index >::getStorageSize( this->offsets );
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getGlobalIndex( const Index segmentIdx, const Index localIdx ) const -> IndexType
{
   if( ! std::is_same< DeviceType, Devices::Host >::value ) {
#ifdef __CUDA_ARCH__
      return offsets[ segmentIdx ] + localIdx;
#else
      return offsets.getElement( segmentIdx ) + localIdx;
#endif
   }
   return offsets[ segmentIdx ] + localIdx;
}

template< typename Device, typename Index, typename Kernel >
__cuda_callable__
auto
CSRView< Device, Index, Kernel >::getSegmentView( const IndexType segmentIdx ) const -> SegmentViewType
{
   return SegmentViewType( segmentIdx, offsets[ segmentIdx ], offsets[ segmentIdx + 1 ] - offsets[ segmentIdx ], 1 );
}

template< typename Device, typename Index, typename Kernel >
template< typename Function >
void
CSRView< Device, Index, Kernel >::forElements( IndexType begin, IndexType end, Function&& f ) const
{
   const auto offsetsView = this->offsets;
   auto l = [ = ] __cuda_callable__( const IndexType segmentIdx ) mutable
   {
      const IndexType begin = offsetsView[ segmentIdx ];
      const IndexType end = offsetsView[ segmentIdx + 1 ];
      IndexType localIdx( 0 );
      for( IndexType globalIdx = begin; globalIdx < end; globalIdx++ )
         f( segmentIdx, localIdx++, globalIdx );
   };
   Algorithms::ParallelFor< Device >::exec( begin, end, l );
}

template< typename Device, typename Index, typename Kernel >
template< typename Function >
void
CSRView< Device, Index, Kernel >::forAllElements( Function&& f ) const
{
   this->forElements( 0, this->getSegmentsCount(), f );
}

template< typename Device, typename Index, typename Kernel >
template< typename Function >
void
CSRView< Device, Index, Kernel >::forSegments( IndexType begin, IndexType end, Function&& function ) const
{
   auto view = this->getConstView();
   auto f = [ = ] __cuda_callable__( IndexType segmentIdx ) mutable
   {
      auto segment = view.getSegmentView( segmentIdx );
      function( segment );
   };
   TNL::Algorithms::ParallelFor< DeviceType >::exec( begin, end, f );
}

template< typename Device, typename Index, typename Kernel >
template< typename Function >
void
CSRView< Device, Index, Kernel >::forAllSegments( Function&& f ) const
{
   this->forSegments( 0, this->getSegmentsCount(), f );
}

template< typename Device, typename Index, typename Kernel >
template< typename Function >
void
CSRView< Device, Index, Kernel >::sequentialForSegments( IndexType begin, IndexType end, Function&& function ) const
{
   for( IndexType i = begin; i < end; i++ )
      forSegments( i, i + 1, function );
}

template< typename Device, typename Index, typename Kernel >
template< typename Function >
void
CSRView< Device, Index, Kernel >::sequentialForAllSegments( Function&& f ) const
{
   this->sequentialForSegments( 0, this->getSegmentsCount(), f );
}

template< typename Device, typename Index, typename Kernel >
template< typename Fetch, typename Reduction, typename ResultKeeper, typename Real >
void
CSRView< Device, Index, Kernel >::reduceSegments( IndexType first,
                                                  IndexType last,
                                                  Fetch& fetch,
                                                  const Reduction& reduction,
                                                  ResultKeeper& keeper,
                                                  const Real& zero ) const
{
   if constexpr( ! std::is_same< DeviceType, Devices::Cuda >::value )
      TNL::Algorithms::Segments::CSRScalarKernel< IndexType, DeviceType >::reduceSegments(
         offsets, first, last, fetch, reduction, keeper, zero );
   else
      kernel.reduceSegments( offsets, first, last, fetch, reduction, keeper, zero );
}

template< typename Device, typename Index, typename Kernel >
template< typename Fetch, typename Reduction, typename ResultKeeper, typename Real >
void
CSRView< Device, Index, Kernel >::reduceAllSegments( Fetch& fetch,
                                                     const Reduction& reduction,
                                                     ResultKeeper& keeper,
                                                     const Real& zero ) const
{
   this->reduceSegments( 0, this->getSegmentsCount(), fetch, reduction, keeper, zero );
}

template< typename Device, typename Index, typename Kernel >
CSRView< Device, Index, Kernel >&
CSRView< Device, Index, Kernel >::operator=( const CSRView& view )
{
   this->offsets.bind( view.offsets );
   this->kernel = view.kernel;
   return *this;
}

template< typename Device, typename Index, typename Kernel >
void
CSRView< Device, Index, Kernel >::save( File& file ) const
{
   file << this->offsets;
}

template< typename Device, typename Index, typename Kernel >
void
CSRView< Device, Index, Kernel >::load( File& file )
{
   file >> this->offsets;
   this->kernel.init( this->offsets );
}

template< typename Device, typename Index, typename Kernel >
template< typename Fetch >
auto
CSRView< Device, Index, Kernel >::print( Fetch&& fetch ) const -> SegmentsPrinter< CSRView, Fetch >
{
   return SegmentsPrinter< CSRView, Fetch >( *this, fetch );
}

}  // namespace Segments
}  // namespace Algorithms
}  // namespace noa::TNL
