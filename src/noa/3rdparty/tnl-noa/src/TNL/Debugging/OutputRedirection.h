// Copyright (c) 2004-2023 Tomáš Oberhuber et al.
//
// This file is part of TNL - Template Numerical Library (https://tnl-project.org/)
//
// SPDX-License-Identifier: MIT

// Implemented by: Jakub Klinkovský

#pragma once

#include <iostream>

#ifndef _MSC_VER
   #include <unistd.h>
#endif

namespace noa::TNL {
namespace Debugging {

class OutputRedirection
{
   int backupFd = -1;
   int targetFd = -1;
   FILE* file = nullptr;

public:
   OutputRedirection() = delete;

   OutputRedirection( int targetFd ) : targetFd( targetFd ) {}

   bool
   redirect( const std::string& fname )
   {
#ifndef _MSC_VER
      // restore the original stream if there is any backup
      if( backupFd >= 0 || file != nullptr )
         if( ! restore() )
            return false;

      // first open the file
      file = ::fopen( fname.c_str(), "w" );
      if( file == nullptr ) {
         std::cerr << "error: fopen() failed, output is not redirected." << std::endl;
         return false;
      }

      // then backup the original file descriptors
      backupFd = ::dup( targetFd );
      if( backupFd < 0 ) {
         std::cerr << "error: dup() failed, output is not redirected." << std::endl;
         return false;
      }

      // finally redirect stdout and stderr
      if( ::dup2( ::fileno( file ), targetFd ) < 0 ) {
         std::cerr << "error: dup2() failed, output is not redirected." << std::endl;
         return false;
      }

      return true;
#else
      std::cerr << "Output redirection is not supported with the MSVC compiler." << std::endl;
      return false;
#endif
   }

   bool
   restore()
   {
#ifndef _MSC_VER
      // first restore the original file descriptor
      if( backupFd >= 0 ) {
         if( ::dup2( backupFd, targetFd ) < 0 ) {
            std::cerr << "error: dup2() failed, output is not restored." << std::endl;
            return false;
         }
         backupFd = -1;
      }

      // then close the file
      if( file != nullptr ) {
         ::fclose( file );
         file = nullptr;
      }
      return true;
#else
      std::cerr << "Output redirection is not supported with the MSVC compiler." << std::endl;
      return false;
#endif
   }

   ~OutputRedirection()
   {
      restore();
   }
};

inline bool
redirect_stdout_stderr( const std::string& stdout_fname, const std::string& stderr_fname, bool restore = false )
{
#ifndef _MSC_VER
   static OutputRedirection stdoutRedir( STDOUT_FILENO );
   static OutputRedirection stderrRedir( STDERR_FILENO );

   if( ! restore ) {
      if( ! stdoutRedir.redirect( stdout_fname ) )
         return false;
      if( ! stderrRedir.redirect( stderr_fname ) )
         return false;
   }
   else {
      stdoutRedir.restore();
      stderrRedir.restore();
   }

   return true;
#else
   std::cerr << "Output redirection is not supported with the MSVC compiler." << std::endl;
   return false;
#endif
}

}  // namespace Debugging
}  // namespace noa::TNL
