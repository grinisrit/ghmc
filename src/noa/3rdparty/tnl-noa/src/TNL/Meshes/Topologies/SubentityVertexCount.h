// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Meshes/Topologies/SubentityVertexMap.h>

namespace noa::TNL {
namespace Meshes {
namespace Topologies {

template< typename EntityTopology, typename SubentityTopology, int SubentityIndex >
struct SubentityVertexCount
{
   static constexpr int count = Subtopology< SubentityTopology, 0 >::count;
};

}  // namespace Topologies
}  // namespace Meshes
}  // namespace noa::TNL