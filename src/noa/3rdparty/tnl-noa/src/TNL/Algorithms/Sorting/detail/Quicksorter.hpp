// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

// Implemented by: Xuan Thang Nguyen, Tomas Oberhuber

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Functional.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Sorting/detail/task.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Sorting/detail/quicksort_kernel.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Sorting/detail/quicksort_1Block.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/Sorting/detail/Quicksorter.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/reduce.h>
#include <noa/3rdparty/tnl-noa/src/TNL/Algorithms/scan.h>

namespace noa::TNL {
namespace Algorithms {
namespace Sorting {

template< typename Value >
template< typename Array, typename Compare >
void
Quicksorter< Value, Devices::Cuda >::sort( Array& arr, const Compare& cmp )
{
#ifdef __CUDACC__
   cudaDeviceProp deviceProp;
   cudaGetDeviceProperties( &deviceProp, 0 );

   /**
    * for every block there is a bit of shared memory reserved, the actual value can slightly differ
    * */
   int sharedReserve = sizeof( int ) * ( 16 + 3 * 32 );
   int maxSharable = deviceProp.sharedMemPerBlock - sharedReserve;

   int blockDim = 512;  // best case

   /**
    * the goal is to use shared memory as often as possible
    * each thread in a block will process n elements, n==multiplier
    * + 1 reserved for pivot (statically allocating Value type throws weird error, hence it needs to be dynamic)
    *
    * blockDim*multiplier*sizeof(Value) + 1*sizeof(Value) <= maxSharable
    * */
   int elemPerBlock =
      ( maxSharable - sizeof( Value ) ) / sizeof( Value );  // try to use up all of shared memory to store elements
   const int maxBlocks = ( 1 << 20 );
   const int maxMultiplier = 8;
   int multiplier = min( elemPerBlock / blockDim, maxMultiplier );

   if( multiplier <= 0 )  // a block cant store 512 elements, sorting some really big data
   {
      blockDim = 256;  // try to fit 256 elements
      multiplier = min( elemPerBlock / blockDim, maxMultiplier );

      if( multiplier <= 0 ) {
         // worst case scenario, shared memory cant be utilized at all because of the sheer size of Value
         // sort has to be done with the use of global memory alone

         this->init( arr, maxBlocks, 512, 0, 0 );
         this->performSort( cmp );
         return;
      }
   }

   TNL_ASSERT_LE( (int) ( blockDim * multiplier * sizeof( Value ) ), maxSharable, "" );

   this->init( arr, maxBlocks, blockDim, multiplier * blockDim, maxSharable );
   this->performSort( cmp );
#endif
}

template< typename Value >
template< typename Array >
void
Quicksorter< Value, Devices::Cuda >::sort( Array& arr )
{
   this->sort( arr,
               [] __cuda_callable__( const Value& a, const Value& b )
               {
                  return a < b;
               } );
}

template< typename Value >
void
Quicksorter< Value, Devices::Cuda >::init( Containers::ArrayView< Value, Devices::Cuda > arr,
                                           int gridDim,
                                           int blockDim,
                                           int desiredElemPerBlock,
                                           int maxSharable )
{
   this->maxBlocks = gridDim;
   this->threadsPerBlock = blockDim;
   this->desiredElemPerBlock = desiredElemPerBlock;
   this->maxSharable = maxSharable;
   this->arr.bind( arr );
   this->auxMem.setSize( arr.getSize() );
   this->aux.bind( auxMem.getView() );
   this->desired_2ndPhasElemPerBlock = desiredElemPerBlock;
   this->maxTasks = min( arr.getSize(), g_maxTasks );
   this->cuda_tasks.setSize( maxTasks );
   this->cuda_newTasks.setSize( maxTasks );
   this->cuda_2ndPhaseTasks.setSize( maxTasks );
   this->cuda_newTasksAmount.setSize( 1 );
   this->cuda_2ndPhaseTasksAmount.setSize( 1 );
   this->cuda_blockToTaskMapping.setSize( maxBlocks );
   this->cuda_reductionTaskInitMem.setSize( maxTasks );

   if( arr.getSize() > desired_2ndPhasElemPerBlock ) {
      cuda_tasks.setElement( 0, TASK( 0, arr.getSize(), 0 ) );
      host_1stPhaseTasksAmount = 1;
   }
   else {
      cuda_2ndPhaseTasks.setElement( 0, TASK( 0, arr.getSize(), 0 ) );
      host_2ndPhaseTasksAmount = 1;
   }

   cuda_2ndPhaseTasksAmount = 0;
}

template< typename Value >
template< typename CMP >
void
Quicksorter< Value, Devices::Cuda >::performSort( const CMP& Cmp )
{
   firstPhase( Cmp );

   int total2ndPhase = host_1stPhaseTasksAmount + host_2ndPhaseTasksAmount;
   if( total2ndPhase > 0 )
      secondPhase( Cmp );

#ifdef CHECK_RESULT_SORT
   if( ! is_sorted( arr ) ) {
      std::ofstream out( "error.txt" );
      out << arr << std::endl;
      out << aux << std::endl;
      out << cuda_tasks << std::endl;
      out << cuda_newTasks << std::endl;
      out << cuda_2ndPhaseTasks << std::endl;

      out << cuda_newTasksAmount << std::endl;
      out << cuda_2ndPhaseTasksAmount << std::endl;

      out << iteration << std::endl;
   }
#endif
}

template< typename Value >
template< typename CMP >
void
Quicksorter< Value, Devices::Cuda >::firstPhase( const CMP& Cmp )
{
   while( host_1stPhaseTasksAmount > 0 ) {
      if( host_1stPhaseTasksAmount >= maxTasks )
         break;

      if( host_2ndPhaseTasksAmount >= maxTasks )  // 2nd phase occupies enoughs tasks to warrant premature 2nd phase sort
      {
         int tmp = host_1stPhaseTasksAmount;
         host_1stPhaseTasksAmount = 0;
         secondPhase( Cmp );
         cuda_2ndPhaseTasksAmount = host_2ndPhaseTasksAmount = 0;
         host_1stPhaseTasksAmount = tmp;
      }

      // just in case newly created tasks wouldnt fit
      // bite the bullet and sort with single blocks
      if( host_1stPhaseTasksAmount * 2 >= maxTasks + ( maxTasks - host_2ndPhaseTasksAmount ) ) {
         if( host_2ndPhaseTasksAmount
             >= 0.75 * maxTasks )  // 2nd phase occupies enoughs tasks to warrant premature 2nd phase sort
         {
            int tmp = host_1stPhaseTasksAmount;
            host_1stPhaseTasksAmount = 0;
            secondPhase( Cmp );
            cuda_2ndPhaseTasksAmount = host_2ndPhaseTasksAmount = 0;
            host_1stPhaseTasksAmount = tmp;
         }
         else
            break;
      }

      //---------------------------------------------------------------

      int elemPerBlock = getElemPerBlock();

      /**
       * initializes tasks so that each block knows which task to work on and which part of array to split
       * also sets pivot needed for partitioning, this is why Cmp is needed
       * */
      int blocksCnt = initTasks( elemPerBlock, Cmp );

      // not enough or too many blocks needed, switch to 2nd phase
      if( blocksCnt <= 1 || blocksCnt > cuda_blockToTaskMapping.getSize() )
         break;

      //-----------------------------------------------
      // do the partitioning

      auto& task = iteration % 2 == 0 ? cuda_tasks : cuda_newTasks;

      Cuda::LaunchConfiguration launch_config;
      launch_config.blockSize.x = threadsPerBlock;
      launch_config.gridSize.x = blocksCnt;
      launch_config.dynamicSharedMemorySize = elemPerBlock * sizeof( Value ) + sizeof( Value );  // elems + 1 for pivot

      /**
       * check if partition procedure can use shared memory for coalesced write after reordering
       *
       * move elements smaller than pivot to the left and bigger to the right
       * note: pivot isnt inserted in the middle yet
       * */
      if( launch_config.dynamicSharedMemorySize <= maxSharable ) {
         constexpr auto kernel = cudaQuickSort1stPhase< Value, CMP, true >;
         Cuda::launchKernelSync( kernel, launch_config, arr, aux, Cmp, elemPerBlock, task, cuda_blockToTaskMapping );
      }
      else {
         launch_config.dynamicSharedMemorySize = sizeof( Value );
         constexpr auto kernel = cudaQuickSort1stPhase< Value, CMP, false >;
         Cuda::launchKernelSync( kernel, launch_config, arr, aux, Cmp, elemPerBlock, task, cuda_blockToTaskMapping );
      }

      /**
       * fill in the gap between smaller and bigger with elements == pivot
       * after writing also create new tasks, each task generates at max 2 tasks
       *
       * tasks smaller than desired_2ndPhasElemPerBlock go into 2nd phase
       * bigger need more blocks to partition and are written into newTask
       * with iteration %2, rotate between the 2 tasks array to save from copying
       * */
      auto& newTask = iteration % 2 == 0 ? cuda_newTasks : cuda_tasks;
      launch_config.gridSize.x = host_1stPhaseTasksAmount;
      launch_config.dynamicSharedMemorySize = sizeof( Value );
      constexpr auto kernel = cudaWritePivot< Value >;
      Cuda::launchKernelSync( kernel,
                              launch_config,
                              arr,
                              aux,
                              desired_2ndPhasElemPerBlock,
                              task,
                              newTask,
                              cuda_newTasksAmount.getData(),
                              cuda_2ndPhaseTasks,
                              cuda_2ndPhaseTasksAmount.getData() );

      //----------------------------------------

      processNewTasks();
      iteration++;
   }
}

template< typename Value >
template< typename CMP >
void
Quicksorter< Value, Devices::Cuda >::secondPhase( const CMP& Cmp )
{
   Cuda::LaunchConfiguration launch_config;
   launch_config.blockSize.x = threadsPerBlock;
   launch_config.gridSize.x = host_1stPhaseTasksAmount + host_2ndPhaseTasksAmount;
   constexpr int stackSize = 32;
   auto& leftoverTasks = iteration % 2 == 0 ? cuda_tasks : cuda_newTasks;

   int elemInShared = desiredElemPerBlock;
   launch_config.dynamicSharedMemorySize =
      elemInShared * sizeof( Value ) + sizeof( Value );  // reserve space for storing elements + 1 pivot
   if( launch_config.dynamicSharedMemorySize > maxSharable ) {
      launch_config.dynamicSharedMemorySize = sizeof( Value );
      elemInShared = 0;
   }

   if( host_1stPhaseTasksAmount > 0 && host_2ndPhaseTasksAmount > 0 ) {
      auto tasks = leftoverTasks.getView( 0, host_1stPhaseTasksAmount );
      auto tasks2 = cuda_2ndPhaseTasks.getView( 0, host_2ndPhaseTasksAmount );

      constexpr auto kernel = cudaQuickSort2ndPhase2< Value, CMP, stackSize >;
      Cuda::launchKernelSync( kernel, launch_config, arr, aux, Cmp, tasks, tasks2, elemInShared, desired_2ndPhasElemPerBlock );
   }
   else if( host_1stPhaseTasksAmount > 0 ) {
      auto tasks = leftoverTasks.getView( 0, host_1stPhaseTasksAmount );
      constexpr auto kernel = cudaQuickSort2ndPhase< Value, CMP, stackSize >;
      Cuda::launchKernelSync( kernel, launch_config, arr, aux, Cmp, tasks, elemInShared, desired_2ndPhasElemPerBlock );
   }
   else {
      auto tasks2 = cuda_2ndPhaseTasks.getView( 0, host_2ndPhaseTasksAmount );
      constexpr auto kernel = cudaQuickSort2ndPhase< Value, CMP, stackSize >;
      Cuda::launchKernelSync( kernel, launch_config, arr, aux, Cmp, tasks2, elemInShared, desired_2ndPhasElemPerBlock );
   }
}

template< typename Value >
int
getSetsNeededFunction( int elemPerBlock, const Quicksorter< Value, Devices::Cuda >& quicksort )
{
   auto view = quicksort.iteration % 2 == 0 ? quicksort.cuda_tasks.getConstView() : quicksort.cuda_newTasks.getConstView();
   auto fetch = [ = ] __cuda_callable__( int i ) -> int
   {
      const auto& task = view[ i ];
      int size = task.partitionEnd - task.partitionBegin;
      return size / elemPerBlock + ( size % elemPerBlock != 0 );
   };
   return reduce< Devices::Cuda >( 0, quicksort.host_1stPhaseTasksAmount, fetch, TNL::Plus{} );
}

template< typename Value >
int
Quicksorter< Value, Devices::Cuda >::getSetsNeeded( int elemPerBlock ) const
{
   return getSetsNeededFunction< Value >( elemPerBlock, *this );
}

template< typename Value >
int
Quicksorter< Value, Devices::Cuda >::getElemPerBlock() const
{
   return desiredElemPerBlock;

   int setsNeeded = getSetsNeeded( desiredElemPerBlock );

   if( setsNeeded <= maxBlocks )
      return desiredElemPerBlock;

   // want multiplier*minElemPerBLock <= x*threadPerBlock
   // find smallest x so that this inequality holds
   double multiplier = 1. * setsNeeded / maxBlocks;
   int elemPerBlock = multiplier * desiredElemPerBlock;
   setsNeeded = elemPerBlock / threadsPerBlock + static_cast< int >( elemPerBlock % threadsPerBlock != 0 );

   return setsNeeded * threadsPerBlock;
}

template< typename Value >
template< typename CMP >
int
Quicksorter< Value, Devices::Cuda >::initTasks( int elemPerBlock, const CMP& Cmp )
{
   auto& src = iteration % 2 == 0 ? arr : aux;
   auto& tasks = iteration % 2 == 0 ? cuda_tasks : cuda_newTasks;

   //--------------------------------------------------------
   Cuda::LaunchConfiguration launch_config;
   launch_config.blockSize.x = threadsPerBlock;
   launch_config.gridSize.x = host_1stPhaseTasksAmount / threadsPerBlock + ( host_1stPhaseTasksAmount % threadsPerBlock != 0 );
   Cuda::launchKernelSync( cudaCalcBlocksNeeded< int >,
                           launch_config,
                           tasks.getView( 0, host_1stPhaseTasksAmount ),
                           elemPerBlock,
                           cuda_reductionTaskInitMem.getView( 0, host_1stPhaseTasksAmount ) );
   // cuda_reductionTaskInitMem[i] == how many blocks task i needs

   inplaceInclusiveScan( cuda_reductionTaskInitMem );
   // cuda_reductionTaskInitMem[i] == how many blocks task [0..i] need

   int blocksNeeded = cuda_reductionTaskInitMem.getElement( host_1stPhaseTasksAmount - 1 );

   // need too many blocks, give back control
   if( blocksNeeded > cuda_blockToTaskMapping.getSize() )
      return blocksNeeded;

   //--------------------------------------------------------
   launch_config.gridSize.x = host_1stPhaseTasksAmount;
   Cuda::launchKernelSync(
      cudaInitTask< Value, CMP >,
      launch_config,
      tasks.getView( 0, host_1stPhaseTasksAmount ),                      // task to read from
      cuda_blockToTaskMapping.getView( 0, blocksNeeded ),                // maps block to a certain task
      cuda_reductionTaskInitMem.getView( 0, host_1stPhaseTasksAmount ),  // has how many each task need blocks precalculated
      src,
      Cmp );  // used to pick pivot

   cuda_newTasksAmount.setElement( 0, 0 );  // resets new element counter
   return blocksNeeded;
}

template< typename Value >
void
Quicksorter< Value, Devices::Cuda >::processNewTasks()
{
   host_1stPhaseTasksAmount = min( cuda_newTasksAmount.getElement( 0 ), maxTasks );
   host_2ndPhaseTasksAmount = min( cuda_2ndPhaseTasksAmount.getElement( 0 ), maxTasks );
}

}  // namespace Sorting
}  // namespace Algorithms
}  // namespace noa::TNL
