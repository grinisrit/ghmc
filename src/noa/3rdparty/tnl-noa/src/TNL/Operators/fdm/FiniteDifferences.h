// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

namespace noa::TNL {
namespace Operators {

template< typename Mesh,
          typename Real,
          typename Index,
          int XDifference,
          int YDifference,
          int ZDifference,
          int XDirection,
          int YDirection,
          int ZDirection >
class FiniteDifferences
{};

}  // namespace Operators
}  // namespace noa::TNL

#include <noa/3rdparty/tnl-noa/src/TNL/Operators/fdm/FiniteDifferences_1D.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Operators/fdm/FiniteDifferences_2D.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Operators/fdm/FiniteDifferences_3D.h>
