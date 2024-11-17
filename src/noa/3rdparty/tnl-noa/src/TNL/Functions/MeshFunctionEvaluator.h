// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Functions/FunctionAdapter.h>

namespace noa::TNL {
namespace Functions {

template< typename OutMeshFunction, typename InFunction, typename Real >
class MeshFunctionEvaluatorTraverserUserData
{
public:
   using InFunctionType = InFunction;

   MeshFunctionEvaluatorTraverserUserData( const InFunction* function,
                                           const Real& time,
                                           OutMeshFunction* meshFunction,
                                           const Real& outFunctionMultiplicator,
                                           const Real& inFunctionMultiplicator )
   : meshFunction( meshFunction ), function( function ), time( time ), outFunctionMultiplicator( outFunctionMultiplicator ),
     inFunctionMultiplicator( inFunctionMultiplicator )
   {}

   OutMeshFunction* meshFunction;
   const InFunction* function;
   const Real time, outFunctionMultiplicator, inFunctionMultiplicator;
};

/***
 * General mesh function evaluator. As an input function any type implementing
 * getValue( meshEntity, time ) may be substituted.
 * Methods:
 *  evaluate() -  evaluate the input function on its domain
 *  evaluateAllEntities() - evaluate the input function on ALL mesh entities
 *  evaluateInteriorEntities() - evaluate the input function only on the INTERIOR mesh entities
 *  evaluateBoundaryEntities() - evaluate the input function only on the BOUNDARY mesh entities
 */
template< typename OutMeshFunction, typename InFunction >
class MeshFunctionEvaluator
{
   static_assert( OutMeshFunction::getDomainDimension() == InFunction::getDomainDimension(),
                  "Input and output functions must have the same domain dimensions." );

public:
   using RealType = typename InFunction::RealType;
   using MeshType = typename OutMeshFunction::MeshType;
   using DeviceType = typename MeshType::DeviceType;
   using TraverserUserData = Functions::MeshFunctionEvaluatorTraverserUserData< OutMeshFunction, InFunction, RealType >;

   template< typename OutMeshFunctionPointer, typename InFunctionPointer >
   static void
   evaluate( OutMeshFunctionPointer& meshFunction,
             const InFunctionPointer& function,
             const RealType& time = (RealType) 0.0,
             const RealType& outFunctionMultiplicator = (RealType) 0.0,
             const RealType& inFunctionMultiplicator = (RealType) 1.0 );

   template< typename OutMeshFunctionPointer, typename InFunctionPointer >
   static void
   evaluateAllEntities( OutMeshFunctionPointer& meshFunction,
                        const InFunctionPointer& function,
                        const RealType& time = (RealType) 0.0,
                        const RealType& outFunctionMultiplicator = (RealType) 0.0,
                        const RealType& inFunctionMultiplicator = (RealType) 1.0 );

   template< typename OutMeshFunctionPointer, typename InFunctionPointer >
   static void
   evaluateInteriorEntities( OutMeshFunctionPointer& meshFunction,
                             const InFunctionPointer& function,
                             const RealType& time = (RealType) 0.0,
                             const RealType& outFunctionMultiplicator = (RealType) 0.0,
                             const RealType& inFunctionMultiplicator = (RealType) 1.0 );

   template< typename OutMeshFunctionPointer, typename InFunctionPointer >
   static void
   evaluateBoundaryEntities( OutMeshFunctionPointer& meshFunction,
                             const InFunctionPointer& function,
                             const RealType& time = (RealType) 0.0,
                             const RealType& outFunctionMultiplicator = (RealType) 0.0,
                             const RealType& inFunctionMultiplicator = (RealType) 1.0 );

protected:
   enum EntitiesType
   {
      all,
      boundary,
      interior
   };

   template< typename OutMeshFunctionPointer, typename InFunctionPointer >
   static void
   evaluateEntities( OutMeshFunctionPointer& meshFunction,
                     const InFunctionPointer& function,
                     const RealType& time,
                     const RealType& outFunctionMultiplicator,
                     const RealType& inFunctionMultiplicator,
                     EntitiesType entitiesType );
};

template< typename MeshType, typename UserData >
class MeshFunctionEvaluatorAssignmentEntitiesProcessor
{
public:
   template< typename EntityType >
   __cuda_callable__
   static inline void
   processEntity( const MeshType& mesh, UserData& userData, const EntityType& entity )
   {
      using FunctionAdapter = FunctionAdapter< MeshType, typename UserData::InFunctionType >;
      ( *userData.meshFunction )( entity ) =
         userData.inFunctionMultiplicator * FunctionAdapter::getValue( *userData.function, entity, userData.time );
      /*cerr << "Idx = " << entity.getIndex()
         << " Value = " << FunctionAdapter::getValue( *userData.function, entity, userData.time )
         << " stored value = " << ( *userData.meshFunction )( entity )
         << " multiplicators = " << std::endl;*/
   }
};

template< typename MeshType, typename UserData >
class MeshFunctionEvaluatorAdditionEntitiesProcessor
{
public:
   template< typename EntityType >
   __cuda_callable__
   static inline void
   processEntity( const MeshType& mesh, UserData& userData, const EntityType& entity )
   {
      using FunctionAdapter = FunctionAdapter< MeshType, typename UserData::InFunctionType >;
      ( *userData.meshFunction )( entity ) =
         userData.outFunctionMultiplicator * ( *userData.meshFunction )( entity )
         + userData.inFunctionMultiplicator * FunctionAdapter::getValue( *userData.function, entity, userData.time );
      /*cerr << "Idx = " << entity.getIndex()
         << " Value = " << FunctionAdapter::getValue( *userData.function, entity, userData.time )
         << " stored value = " << ( *userData.meshFunction )( entity )
         << " multiplicators = " << std::endl;*/
   }
};

}  // namespace Functions
}  // namespace noa::TNL

#include <noa/3rdparty/tnl-noa/src/TNL/Functions/MeshFunctionEvaluator_impl.h>
