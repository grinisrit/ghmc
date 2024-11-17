// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Config/ParameterContainer.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Containers/StaticVector.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Functions/Domain.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Devices/Cuda.h>

namespace noa::TNL {
namespace Functions {
namespace Analytic {

template< typename Real, int Dimension >
class BlobBase : public Domain< Dimension, SpaceDomain >
{
public:
   using RealType = Real;

   bool
   setup( const Config::ParameterContainer& parameters, const String& prefix = "" );

protected:
   RealType height;
};

template< int Dimension, typename Real >
class Blob
{};

template< typename Real >
class Blob< 1, Real > : public BlobBase< Real, 1 >
{
public:
   enum
   {
      Dimension = 1
   };
   using RealType = Real;
   using PointType = Containers::StaticVector< Dimension, Real >;

   Blob();

   template< int XDiffOrder = 0, int YDiffOrder = 0, int ZDiffOrder = 0 >
   __cuda_callable__
   RealType
   getPartialDerivative( const PointType& v, const Real& time = 0.0 ) const;

   __cuda_callable__
   RealType
   operator()( const PointType& v, const Real& time = 0.0 ) const;
};

template< typename Real >
class Blob< 2, Real > : public BlobBase< Real, 2 >
{
public:
   enum
   {
      Dimension = 2
   };
   using RealType = Real;
   using PointType = Containers::StaticVector< Dimension, Real >;

   Blob();

   template< int XDiffOrder = 0, int YDiffOrder = 0, int ZDiffOrder = 0 >
   __cuda_callable__
   RealType
   getPartialDerivative( const PointType& v, const Real& time = 0.0 ) const;

   __cuda_callable__
   RealType
   operator()( const PointType& v, const Real& time = 0.0 ) const;
};

template< typename Real >
class Blob< 3, Real > : public BlobBase< Real, 3 >
{
public:
   enum
   {
      Dimension = 3
   };
   using RealType = Real;
   using PointType = Containers::StaticVector< Dimension, Real >;

   Blob();

   template< int XDiffOrder = 0, int YDiffOrder = 0, int ZDiffOrder = 0 >
   __cuda_callable__
   RealType
   getPartialDerivative( const PointType& v, const Real& time = 0.0 ) const;

   __cuda_callable__
   RealType
   operator()( const PointType& v, const Real& time = 0.0 ) const;
};

template< int Dimension, typename Real >
std::ostream&
operator<<( std::ostream& str, const Blob< Dimension, Real >& f )
{
   str << "Level-set pseudo square function.";
   return str;
}

}  // namespace Analytic
}  // namespace Functions
}  // namespace noa::TNL

#include <noa/3rdparty/tnl-noa/src/TNL/Functions/Analytic/Blob_impl.h>
