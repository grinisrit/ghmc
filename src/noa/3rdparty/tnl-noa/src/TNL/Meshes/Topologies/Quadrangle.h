// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Meshes/Topologies/Edge.h>

namespace noa::TNL {
namespace Meshes {
namespace Topologies {

struct Quadrangle
{
   static constexpr int dimension = 2;
};

template<>
struct Subtopology< Quadrangle, 0 >
{
   using Topology = Vertex;

   static constexpr int count = 4;
};

template<>
struct Subtopology< Quadrangle, 1 >
{
   using Topology = Edge;

   static constexpr int count = 4;
};

/****
 * Indexing of the vertices follows the VTK file format
 *
 *   3                     2
 *    +-------------------+
 *    |                   |
 *    |                   |
 *    |                   |
 *    |                   |
 *    |                   |
 *    +-------------------+
 *   0                     1
 *
 * The edges are indexed as follows:
 *
 *              2
 *    +-------------------+
 *    |                   |
 *    |                   |
 *  3 |                   | 1
 *    |                   |
 *    |                   |
 *    +-------------------+
 *              0
 *
 */

template<>
struct SubentityVertexMap< Quadrangle, Edge, 0, 0 >
{
   static constexpr int index = 0;
};
template<>
struct SubentityVertexMap< Quadrangle, Edge, 0, 1 >
{
   static constexpr int index = 1;
};

template<>
struct SubentityVertexMap< Quadrangle, Edge, 1, 0 >
{
   static constexpr int index = 1;
};
template<>
struct SubentityVertexMap< Quadrangle, Edge, 1, 1 >
{
   static constexpr int index = 2;
};

template<>
struct SubentityVertexMap< Quadrangle, Edge, 2, 0 >
{
   static constexpr int index = 2;
};
template<>
struct SubentityVertexMap< Quadrangle, Edge, 2, 1 >
{
   static constexpr int index = 3;
};

template<>
struct SubentityVertexMap< Quadrangle, Edge, 3, 0 >
{
   static constexpr int index = 3;
};
template<>
struct SubentityVertexMap< Quadrangle, Edge, 3, 1 >
{
   static constexpr int index = 0;
};

}  // namespace Topologies
}  // namespace Meshes
}  // namespace noa::TNL
