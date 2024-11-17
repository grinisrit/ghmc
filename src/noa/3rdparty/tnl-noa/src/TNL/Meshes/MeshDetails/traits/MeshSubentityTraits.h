// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Containers/StaticArray.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Meshes/MeshDetails/traits/MeshEntityTraits.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Meshes/Topologies/SubentityVertexMap.h>

namespace noa::TNL {
namespace Meshes {

/****
 *       Mesh subentity traits with specializations
 *
 *  DYNAMIC TOPOLOGY
 *       FALSE
 */
template< typename MeshConfig, typename Device, typename EntityTopology, int Dimension >
class MeshSubentityTraits< MeshConfig, Device, EntityTopology, Dimension, false >
{
   using GlobalIndexType = typename MeshConfig::GlobalIndexType;
   using LocalIndexType = typename MeshConfig::LocalIndexType;

public:
   static_assert( 0 <= Dimension && Dimension <= MeshConfig::meshDimension, "invalid dimension" );
   static_assert( EntityTopology::dimension > Dimension, "Subentity dimension must be smaller than the entity dimension." );

   static constexpr bool storageEnabled = MeshConfig::subentityStorage( EntityTopology::dimension, Dimension );
   static constexpr int count = Topologies::Subtopology< EntityTopology, Dimension >::count;

   using SubentityTopology = typename MeshEntityTraits< MeshConfig, Device, Dimension >::EntityTopology;
   using SubentityType = typename MeshEntityTraits< MeshConfig, Device, Dimension >::EntityType;

   template< LocalIndexType subentityIndex, LocalIndexType subentityVertexIndex >
   struct Vertex
   {
      static constexpr int index =
         Topologies::SubentityVertexMap< EntityTopology, SubentityTopology, subentityIndex, subentityVertexIndex >::index;
   };
};

/****
 *       Mesh subentity traits with specializations
 *
 *  DYNAMIC TOPOLOGY
 *       TRUE
 */
template< typename MeshConfig, typename Device, typename EntityTopology, int Dimension >
class MeshSubentityTraits< MeshConfig, Device, EntityTopology, Dimension, true >
{
   using GlobalIndexType = typename MeshConfig::GlobalIndexType;
   using LocalIndexType = typename MeshConfig::LocalIndexType;

public:
   static_assert( 0 <= Dimension && Dimension <= MeshConfig::meshDimension, "invalid dimension" );
   static_assert( EntityTopology::dimension > Dimension, "Subentity dimension must be smaller than the entity dimension." );

   static constexpr bool storageEnabled = MeshConfig::subentityStorage( EntityTopology::dimension, Dimension );

   using SubentityTopology = typename MeshEntityTraits< MeshConfig, Device, Dimension >::EntityTopology;
   using SubentityType = typename MeshEntityTraits< MeshConfig, Device, Dimension >::EntityType;
};

}  // namespace Meshes
}  // namespace noa::TNL
