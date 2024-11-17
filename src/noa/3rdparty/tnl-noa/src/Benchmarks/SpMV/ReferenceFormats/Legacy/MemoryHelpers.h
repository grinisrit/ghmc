// Copyright (c) 2004-2022 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>

#include <TNL/Cuda/CheckDevice.h>
#include <TNL/Exceptions/CudaSupportMissing.h>
#include <TNL/Exceptions/CudaBadAlloc.h>

namespace TNL {
namespace Cuda {

template< typename ObjectType >
//[[deprecated( "Allocators::Cuda and MultiDeviceMemoryOperations should be used instead." )]]
ObjectType*
passToDevice( const ObjectType& object )
{
#ifdef __CUDACC__
   ObjectType* deviceObject;
   if( cudaMalloc( (void**) &deviceObject, (size_t) sizeof( ObjectType ) ) != cudaSuccess )
      throw Exceptions::CudaBadAlloc();
   if( cudaMemcpy( (void*) deviceObject, (void*) &object, sizeof( ObjectType ), cudaMemcpyHostToDevice ) != cudaSuccess ) {
      TNL_CHECK_CUDA_DEVICE;
      cudaFree( (void*) deviceObject );
      TNL_CHECK_CUDA_DEVICE;
      return 0;
   }
   return deviceObject;
#else
   throw Exceptions::CudaSupportMissing();
#endif
}

template< typename ObjectType >
//[[deprecated( "Allocators::Cuda should be used instead." )]]
void
freeFromDevice( ObjectType* deviceObject )
{
#ifdef __CUDACC__
   cudaFree( (void*) deviceObject );
   TNL_CHECK_CUDA_DEVICE;
#else
   throw Exceptions::CudaSupportMissing();
#endif
}

}  // namespace Cuda
}  // namespace TNL
