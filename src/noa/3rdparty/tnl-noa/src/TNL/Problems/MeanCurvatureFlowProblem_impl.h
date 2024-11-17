// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

/***
 * Authors:
 * Oberhuber Tomas, tomas.oberhuber@fjfi.cvut.cz
 * Szekely Ondrej, ondra.szekely@gmail.com
 */

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/FileName.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/MatrixSetter.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Matrices/MultidiagonalMatrixSetter.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Logger.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Solvers/PDE/ExplicitUpdater.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Solvers/PDE/BoundaryConditionsSetter.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Solvers/PDE/LinearSystemAssembler.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Solvers/PDE/BackwardTimeDiscretisation.h>

#include "MeanCurvatureFlowProblem.h"

namespace noa::TNL {
namespace Problems {

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
String
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::getPrologHeader() const
{
   return String( "Mean Curvative Flow" );
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
void
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::writeProlog(
   Logger& logger,
   const Config::ParameterContainer& parameters ) const
{}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
bool
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::setup(
   const Config::ParameterContainer& parameters )
{
   if( ! this->boundaryCondition.setup( parameters, "boundary-conditions-" )
       || ! this->rightHandSide.setup( parameters, "right-hand-side-" ) )
      return false;
   this->differentialOperator.nonlinearDiffusionOperator.operatorQ.setEps( parameters.getParameter< double >( "eps" ) );
   return true;
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
typename MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::IndexType
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::getDofs( const MeshType& mesh ) const
{
   /****
    * Set-up DOFs and supporting grid functions
    */
   return mesh.template getEntitiesCount< typename Mesh::Cell >();
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
void
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::bindDofs( const MeshType& mesh,
                                                                                                    DofVectorType& dofVector )
{
   const IndexType dofs = mesh.template getEntitiesCount< typename Mesh::Cell >();
   this->solution.bind( dofVector.getData(), dofs );
   // differentialOperator.nonlinearDiffusionOperator.operatorQ.bind(solution);
   //   this->differentialOperator.setupDofs(mesh);
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
bool
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::setInitialCondition(
   const Config::ParameterContainer& parameters,
   const MeshType& mesh,
   DofVectorType& dofs,
   MeshDependentDataPointer& meshDependentData )
{
   this->bindDofs( mesh, dofs );
   const String& initialConditionFile = parameters.getParameter< String >( "initial-condition" );
   if( ! this->solution.load( initialConditionFile ) ) {
      std::cerr << "I am not able to load the initial condition from the file " << initialConditionFile << "." << std::endl;
      return false;
   }
   return true;
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
template< typename Matrix >
bool
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::setupLinearSystem(
   const MeshType& mesh,
   Matrix& matrix )
{
   const IndexType dofs = this->getDofs( mesh );
   typedef typename MatrixType::RowsCapacitiesType RowsCapacitiesTypeType;
   RowsCapacitiesTypeType rowLengths;
   rowLengths.setSize( dofs );
   MatrixSetter< MeshType, DifferentialOperator, BoundaryCondition, RowsCapacitiesTypeType > matrixSetter;
   matrixSetter.template getCompressedRowLengths< typename Mesh::Cell >(
      mesh, differentialOperator, boundaryCondition, rowLengths );
   matrix.setDimensions( dofs, dofs );
   matrix.setRowCapacities( rowLengths );
   return true;
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
bool
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::makeSnapshot(
   const RealType& time,
   const IndexType& step,
   const MeshType& mesh,
   DofVectorType& dofs,
   MeshDependentDataPointer& meshDependentData )
{
   std::cout << std::endl << "Writing output at time " << time << " step " << step << "." << std::endl;

   this->bindDofs( mesh, dofs );
   // cout << "dofs = " << dofs << std::endl;
   String fileName;
   FileNameBaseNumberEnding( "u-", step, 5, ".vti", fileName );
   if( ! this->solution.write( "u", fileName ) )
      return false;
   return true;
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
void
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::getExplicitUpdate(
   const RealType& time,
   const RealType& tau,
   const MeshType& mesh,
   DofVectorType& inDofs,
   DofVectorType& outDofs,
   MeshDependentDataPointer& meshDependentData )
{
   /****
    * If you use an explicit solver like Euler or Merson, you
    * need to implement this method. Compute the right-hand side of
    *
    *   d/dt u(x) = fu( x, u )
    *
    * You may use supporting vectors again if you need.
    */

   //   this->differentialOperator.computeFirstGradient(mesh,time,u);

   // cout << "u = " << u << std::endl;
   // this->bindDofs( mesh, u );
   MeshFunctionType u( mesh, inDofs );
   MeshFunctionType fu( mesh, outDofs );
   // differentialOperator.nonlinearDiffusionOperator.operatorQ.update( mesh, time );
   ExplicitUpdater< Mesh, MeshFunctionType, DifferentialOperator, BoundaryCondition, RightHandSide > explicitUpdater;
   explicitUpdater.setDifferentialOperator( this->differentialOperatorPointer );
   explicitUpdater.setBoundaryConditions( this->boundaryConditionPointer );
   explicitUpdater.setRightHandSide( this->rightHandSidePointer );

   explicitUpdater.template update< typename Mesh::Cell >( time, tau, mesh, u, fu );

   /*cout << "u = " << u << std::endl;
  std::cout << "fu = " << fu << std::endl;
   u.save( "u.tnl" );
   fu.save( "fu.tnl" );
   getchar();*/
}

template< typename Mesh, typename BoundaryCondition, typename RightHandSide, typename DifferentialOperator >
template< typename Matrix >
void
MeanCurvatureFlowProblem< Mesh, BoundaryCondition, RightHandSide, DifferentialOperator >::assemblyLinearSystem(
   const RealType& time,
   const RealType& tau,
   const MeshType& mesh,
   DofVectorType& dofsU,
   Matrix& matrix,
   DofVectorType& b,
   MeshDependentDataPointer& meshDependentData )
{
   MeshFunctionType u( mesh, dofsU );
   LinearSystemAssembler< Mesh,
                          MeshFunctionType,
                          DifferentialOperator,
                          BoundaryCondition,
                          RightHandSide,
                          BackwardTimeDiscretisation,
                          MatrixType,
                          DofVectorType >
      systemAssembler;
   systemAssembler.template assembly< typename Mesh::Cell >(
      time, tau, mesh, this->differentialOperator, this->boundaryCondition, this->rightHandSide, u, matrix, b );
   /*matrix.print(std::cout );
  std::cout << std::endl << b << std::endl;
  std::cout << std::endl << u << std::endl;
   getchar();
   //abort();*/
}

}  // namespace Problems
}  // namespace noa::TNL
