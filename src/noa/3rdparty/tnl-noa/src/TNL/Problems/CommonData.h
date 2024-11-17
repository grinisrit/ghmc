// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

// Implemented by: Tomas Oberhuber

#pragma once

#include <noa/3rdparty/tnl-noa/src/TNL/Config/ParameterContainer.h>

namespace noa::TNL {
namespace Problems {

class CommonData
{
public:
   bool
   setup( const Config::ParameterContainer& parameters )
   {
      return true;
   }
};

}  // namespace Problems
}  // namespace noa::TNL
