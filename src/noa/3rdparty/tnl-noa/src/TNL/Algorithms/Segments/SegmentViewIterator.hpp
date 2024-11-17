// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Segments/SegmentView.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Assert.h>

namespace noa::TNL {
namespace Algorithms {
namespace Segments {

template< typename SegmentView >
__cuda_callable__
SegmentViewIterator< SegmentView >::SegmentViewIterator( const SegmentViewType& segmentView, const IndexType& localIdx )
: segmentView( segmentView ), localIdx( localIdx )
{}

template< typename SegmentView >
__cuda_callable__
bool
SegmentViewIterator< SegmentView >::operator==( const SegmentViewIterator& other ) const
{
   return &this->segmentView == &other.segmentView && localIdx == other.localIdx;
}

template< typename SegmentView >
__cuda_callable__
bool
SegmentViewIterator< SegmentView >::operator!=( const SegmentViewIterator& other ) const
{
   return ! ( other == *this );
}

template< typename SegmentView >
__cuda_callable__
SegmentViewIterator< SegmentView >&
SegmentViewIterator< SegmentView >::operator++()
{
   if( localIdx < segmentView.getSize() )
      localIdx++;
   return *this;
}

template< typename SegmentView >
__cuda_callable__
SegmentViewIterator< SegmentView >&
SegmentViewIterator< SegmentView >::operator--()
{
   if( localIdx > 0 )
      localIdx--;
   return *this;
}

template< typename SegmentView >
__cuda_callable__
auto
SegmentViewIterator< SegmentView >::operator*() const -> const SegmentElementType
{
   return SegmentElementType(
      this->segmentView.getSegmentIndex(), this->localIdx, this->segmentView.getGlobalIndex( this->localIdx ) );
}

}  // namespace Segments
}  // namespace Algorithms
}  // namespace noa::TNL
