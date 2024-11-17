// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

// Implemented by: Xuan Thang Nguyen, Tomas Oberhuber

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Sorting/detail/Quicksorter.h>

namespace noa::TNL {
namespace Algorithms {
namespace Sorting {

struct Quicksort
{
   template< typename Array >
   void static sort( Array& array )
   {
      Quicksorter< typename Array::ValueType, typename Array::DeviceType > qs;
      qs.sort( array );
   }

   template< typename Array, typename Compare >
   void static sort( Array& array, const Compare& compare )
   {
      Quicksorter< typename Array::ValueType, typename Array::DeviceType > qs;
      qs.sort( array, compare );
   }
};

}  // namespace Sorting
}  // namespace Algorithms
}  // namespace noa::TNL
