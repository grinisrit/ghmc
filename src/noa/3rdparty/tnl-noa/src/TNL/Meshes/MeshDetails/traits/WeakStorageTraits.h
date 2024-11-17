// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Meshes/MeshDetails/traits/MeshSubentityTraits.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Meshes/MeshDetails/traits/MeshSuperentityTraits.h>

namespace noa::TNL {
namespace Meshes {

template< typename MeshConfig,
          typename Device,
          typename EntityTopology,
          typename SubdimensionTag,
          bool sensible = ( SubdimensionTag::value < EntityTopology::dimension ) >
struct WeakSubentityStorageTrait
{
   static constexpr bool storageEnabled =
      MeshTraits< MeshConfig, Device >::template SubentityTraits< EntityTopology, SubdimensionTag::value >::storageEnabled;
};

template< typename MeshConfig, typename Device, typename EntityTopology, typename SubdimensionTag >
struct WeakSubentityStorageTrait< MeshConfig, Device, EntityTopology, SubdimensionTag, false >
{
   static constexpr bool storageEnabled = false;
};

template< typename MeshConfig,
          typename Device,
          typename EntityTopology,
          typename SuperdimensionTag,
          bool sensible = ( SuperdimensionTag::value > EntityTopology::dimension ) >
struct WeakSuperentityStorageTrait
{
   static constexpr bool storageEnabled =
      MeshTraits< MeshConfig, Device >::template SuperentityTraits< EntityTopology, SuperdimensionTag::value >::storageEnabled;
};

template< typename MeshConfig, typename Device, typename EntityTopology, typename SuperdimensionTag >
struct WeakSuperentityStorageTrait< MeshConfig, Device, EntityTopology, SuperdimensionTag, false >
{
   static constexpr bool storageEnabled = false;
};

}  // namespace Meshes
}  // namespace noa::TNL
