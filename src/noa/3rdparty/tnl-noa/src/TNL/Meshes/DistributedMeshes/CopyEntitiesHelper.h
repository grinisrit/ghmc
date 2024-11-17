// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/ParallelFor.h>

namespace noa::TNL {
namespace Meshes {
namespace DistributedMeshes {

template< typename MeshFunctionType, int dim = MeshFunctionType::getMeshDimension() >
class CopyEntitiesHelper;

template< typename MeshFunctionType >
class CopyEntitiesHelper< MeshFunctionType, 1 >
{
public:
   using CoordinatesType = typename MeshFunctionType::MeshType::CoordinatesType;
   using Cell = typename MeshFunctionType::MeshType::Cell;
   using Index = typename MeshFunctionType::MeshType::GlobalIndexType;

   template< typename FromFunction >
   static void
   Copy( FromFunction& from, MeshFunctionType& to, CoordinatesType& fromBegin, CoordinatesType& toBegin, CoordinatesType& size )
   {
      auto toData = to.getData().getData();
      auto fromData = from.getData().getData();
      auto* fromMesh = &from.getMeshPointer().template getData< typename MeshFunctionType::MeshType::DeviceType >();
      auto* toMesh = &to.getMeshPointer().template getData< typename MeshFunctionType::MeshType::DeviceType >();
      auto kernel = [ fromData, toData, fromMesh, toMesh, fromBegin, toBegin ] __cuda_callable__( Index i )
      {
         Cell fromEntity( *fromMesh );
         Cell toEntity( *toMesh );
         toEntity.getCoordinates().x() = toBegin.x() + i;
         toEntity.refresh();
         fromEntity.getCoordinates().x() = fromBegin.x() + i;
         fromEntity.refresh();
         toData[ toEntity.getIndex() ] = fromData[ fromEntity.getIndex() ];
      };
      Algorithms::ParallelFor< typename MeshFunctionType::MeshType::DeviceType >::exec( (Index) 0, (Index) size.x(), kernel );
   }
};

template< typename MeshFunctionType >

class CopyEntitiesHelper< MeshFunctionType, 2 >
{
public:
   using CoordinatesType = typename MeshFunctionType::MeshType::CoordinatesType;
   using Cell = typename MeshFunctionType::MeshType::Cell;
   using Index = typename MeshFunctionType::MeshType::GlobalIndexType;

   template< typename FromFunction >
   static void
   Copy( FromFunction& from, MeshFunctionType& to, CoordinatesType& fromBegin, CoordinatesType& toBegin, CoordinatesType& size )
   {
      auto toData = to.getData().getData();
      auto fromData = from.getData().getData();
      auto* fromMesh = &from.getMeshPointer().template getData< typename MeshFunctionType::MeshType::DeviceType >();
      auto* toMesh = &to.getMeshPointer().template getData< typename MeshFunctionType::MeshType::DeviceType >();
      auto kernel = [ fromData, toData, fromMesh, toMesh, fromBegin, toBegin ] __cuda_callable__( Index i, Index j )
      {
         Cell fromEntity( *fromMesh );
         Cell toEntity( *toMesh );
         toEntity.getCoordinates().x() = toBegin.x() + i;
         toEntity.getCoordinates().y() = toBegin.y() + j;
         toEntity.refresh();
         fromEntity.getCoordinates().x() = fromBegin.x() + i;
         fromEntity.getCoordinates().y() = fromBegin.y() + j;
         fromEntity.refresh();
         toData[ toEntity.getIndex() ] = fromData[ fromEntity.getIndex() ];
      };
      Algorithms::ParallelFor2D< typename MeshFunctionType::MeshType::DeviceType >::exec(
         (Index) 0, (Index) 0, (Index) size.x(), (Index) size.y(), kernel );
   }
};

template< typename MeshFunctionType >
class CopyEntitiesHelper< MeshFunctionType, 3 >
{
public:
   using CoordinatesType = typename MeshFunctionType::MeshType::CoordinatesType;
   using Cell = typename MeshFunctionType::MeshType::Cell;
   using Index = typename MeshFunctionType::MeshType::GlobalIndexType;

   template< typename FromFunction >
   static void
   Copy( FromFunction& from, MeshFunctionType& to, CoordinatesType& fromBegin, CoordinatesType& toBegin, CoordinatesType& size )
   {
      auto toData = to.getData().getData();
      auto fromData = from.getData().getData();
      auto* fromMesh = &from.getMeshPointer().template getData< typename MeshFunctionType::MeshType::DeviceType >();
      auto* toMesh = &to.getMeshPointer().template getData< typename MeshFunctionType::MeshType::DeviceType >();
      auto kernel = [ fromData, toData, fromMesh, toMesh, fromBegin, toBegin ] __cuda_callable__( Index i, Index j, Index k )
      {
         Cell fromEntity( *fromMesh );
         Cell toEntity( *toMesh );
         toEntity.getCoordinates().x() = toBegin.x() + i;
         toEntity.getCoordinates().y() = toBegin.y() + j;
         toEntity.getCoordinates().z() = toBegin.z() + k;
         toEntity.refresh();
         fromEntity.getCoordinates().x() = fromBegin.x() + i;
         fromEntity.getCoordinates().y() = fromBegin.y() + j;
         fromEntity.getCoordinates().z() = fromBegin.z() + k;
         fromEntity.refresh();
         toData[ toEntity.getIndex() ] = fromData[ fromEntity.getIndex() ];
      };
      Algorithms::ParallelFor3D< typename MeshFunctionType::MeshType::DeviceType >::exec(
         (Index) 0, (Index) 0, (Index) 0, (Index) size.x(), (Index) size.y(), (Index) size.z(), kernel );
   }
};

}  // namespace DistributedMeshes
}  // namespace Meshes
}  // namespace noa::TNL
